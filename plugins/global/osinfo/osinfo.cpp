/*
    Copyright (c) 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "osinfo.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QIcon>

#include <KPluginFactory>
#include <KLocalizedString>
#include <KCoreAddons>
#include <KOSRelease>

#include <SensorContainer.h>

// Uppercase the first letter of each word.
QString upperCaseFirst(const QString &input)
{
    auto parts = input.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (auto &part : parts) {
        part[0] = part[0].toUpper();
    }
    return parts.join(QLatin1Char(' '));
}

// Helper to simplify async dbus calls.
template <typename T>
QDBusPendingCallWatcher *dbusCall(
    const QDBusConnection &bus,
    const QString &service,
    const QString &object,
    const QString &interface,
    const QString &method,
    const QVariantList &arguments,
    std::function<void(const QDBusPendingReply<T>&)> callback
)
{
    auto message = QDBusMessage::createMethodCall(service, object, interface, method);
    message.setArguments(arguments);
    auto watcher = new QDBusPendingCallWatcher{bus.asyncCall(message)};
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, watcher, [callback](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<T> reply = watcher->reply();
        callback(reply);
        watcher->deleteLater();
    });
    return watcher;
}

class OSInfoPrivate
{
public:
    OSInfoPrivate(OSInfoPlugin *qq);
    virtual ~OSInfoPrivate() = default;

    virtual void update();

    OSInfoPlugin *q;

    SensorContainer *container = nullptr;

    SensorObject *kernelObject = nullptr;
    SensorProperty *kernelNameProperty = nullptr;
    SensorProperty *kernelVersionProperty = nullptr;
    SensorProperty *kernelPrettyNameProperty = nullptr;

    SensorObject *systemObject = nullptr;
    SensorProperty *hostnameProperty = nullptr;
    SensorProperty *osNameProperty = nullptr;
    SensorProperty *osVersionProperty = nullptr;
    SensorProperty *osPrettyNameProperty = nullptr;
    SensorProperty *osLogoProperty = nullptr;
    SensorProperty *osUrlProperty = nullptr;

    SensorObject *plasmaObject = nullptr;
    SensorProperty *qtVersionProperty = nullptr;
    SensorProperty *kfVersionProperty = nullptr;
    SensorProperty *plasmaVersionProperty = nullptr;
};

class LinuxPrivate : public OSInfoPrivate
{
public:
    LinuxPrivate(OSInfoPlugin *qq) : OSInfoPrivate(qq) { }

    void update() override;
};

OSInfoPrivate::OSInfoPrivate(OSInfoPlugin *qq)
    : q(qq)
{
    container = new SensorContainer(QStringLiteral("os"), i18nc("@title", "Operating System"), q);

    kernelObject = new SensorObject(QStringLiteral("kernel"), i18nc("@title", "Kernel"), container);
    kernelNameProperty = new SensorProperty(QStringLiteral("name"), i18nc("@title", "Kernel Name"), QString{}, kernelObject);
    kernelVersionProperty = new SensorProperty(QStringLiteral("version"), i18nc("@title", "Kernel Version"), QString{}, kernelObject);
    kernelPrettyNameProperty = new SensorProperty(QStringLiteral("prettyName"), i18nc("@title", "Kernel Name and Version"), QString{}, kernelObject);
    kernelPrettyNameProperty->setShortName(i18nc("@title Kernel Name and Version", "Kernel"));

    systemObject = new SensorObject(QStringLiteral("system"), i18nc("@title", "System"), container);
    hostnameProperty = new SensorProperty(QStringLiteral("hostname"), i18nc("@title", "Hostname"), QString{}, systemObject);
    osNameProperty = new SensorProperty(QStringLiteral("name"), i18nc("@title", "Operating System Name"), QString{}, systemObject);
    osVersionProperty = new SensorProperty(QStringLiteral("version"), i18nc("@title", "Operating System Version"), QString{}, systemObject);
    osPrettyNameProperty = new SensorProperty(QStringLiteral("prettyName"), i18nc("@title", "Operating System Name and Version"), QString{}, systemObject);
    osPrettyNameProperty->setShortName(i18nc("@title Operating System Name and Version", "OS"));
    osLogoProperty = new SensorProperty(QStringLiteral("logo"), i18nc("@title", "Operating System Logo"), QString{}, systemObject);
    osUrlProperty = new SensorProperty(QStringLiteral("url"), i18nc("@title", "Operating System URL"), QString{}, systemObject);

    plasmaObject = new SensorObject(QStringLiteral("plasma"), i18nc("@title", "KDE Plasma"), container);
    qtVersionProperty = new SensorProperty(QStringLiteral("qtVersion"), i18nc("@title", "Qt Version"), QString{}, plasmaObject);
    kfVersionProperty = new SensorProperty(QStringLiteral("kfVersion"), i18nc("@title", "KDE Frameworks Version"), QString{}, plasmaObject);
    plasmaVersionProperty = new SensorProperty(QStringLiteral("plasmaVersion"), i18nc("@title", "KDE Plasma Version"), QString{}, plasmaObject);
}

OSInfoPlugin::~OSInfoPlugin() = default;

void OSInfoPrivate::update()
{
    auto kernelName = upperCaseFirst(QSysInfo::kernelType());
    kernelNameProperty->setValue(kernelName);
    kernelVersionProperty->setValue(QSysInfo::kernelVersion());
    kernelPrettyNameProperty->setValue(QString{kernelName % QLatin1Char(' ') % QSysInfo::kernelVersion()});
    hostnameProperty->setValue(QSysInfo::machineHostName());

    KOSRelease os;
    osNameProperty->setValue(os.name());
    osVersionProperty->setValue(os.version());
    osPrettyNameProperty->setValue(os.prettyName());
    osLogoProperty->setValue(os.logo());
    osUrlProperty->setValue(os.homeUrl());

    qtVersionProperty->setValue(QString::fromLatin1(qVersion()));
    kfVersionProperty->setValue(KCoreAddons::versionString());

    dbusCall<QVariant>(
        QDBusConnection::sessionBus(),
        QStringLiteral("org.kde.plasmashell"),
        QStringLiteral("/MainApplication"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("Get"),
        { QStringLiteral("org.qtproject.Qt.QCoreApplication"), QStringLiteral("applicationVersion") },
        [this](const QDBusPendingReply<QVariant> &reply) {
            if (reply.isError()) {
                qWarning() << "Could not determine Plasma version, got: " << reply.error().message();
                plasmaVersionProperty->setValue(i18nc("@info", "Unknown"));
            } else {
                plasmaVersionProperty->setValue(reply.value());
            }
        }
    );
}

void LinuxPrivate::update()
{
    OSInfoPrivate::update();

    // Override some properties with values from hostnamed, if available.
    dbusCall<QVariantMap>(
        QDBusConnection::systemBus(),
        QStringLiteral("org.freedesktop.hostname1"),
        QStringLiteral("/org/freedesktop/hostname1"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("GetAll"),
        { QStringLiteral("org.freedesktop.hostname1") },
        [this](const QDBusPendingReply<QVariantMap> &reply) {
            if (reply.isError()) {
                qWarning() << "Could not contact hostnamed, got: " << reply.error().message();
            } else {
                auto properties = reply.value();
                auto kernelName = properties.value(QStringLiteral("KernelName"), kernelNameProperty->value()).toString();
                kernelNameProperty->setValue(kernelName);
                auto kernelVersion = properties.value(QStringLiteral("KernelRelease"), kernelVersionProperty->value()).toString();
                kernelVersionProperty->setValue(kernelVersion);
                kernelPrettyNameProperty->setValue(QString{kernelName % QLatin1Char(' ') % kernelVersion});

                auto prettyHostName = properties.value(QStringLiteral("PrettyHostname"), QString{}).toString();
                if (!prettyHostName.isEmpty()) {
                    hostnameProperty->setValue(prettyHostName);
                } else {
                    hostnameProperty->setValue(properties.value(QStringLiteral("Hostname"), hostnameProperty->value()));
                }
            }
        }
    );
}

OSInfoPlugin::OSInfoPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
#ifdef Q_OS_LINUX
    d = std::make_unique<LinuxPrivate>(this);
#else
    d = std::make_unique<OSInfoPrivate>(this);
#endif
    d->update();
}

K_PLUGIN_FACTORY(PluginFactory, registerPlugin<OSInfoPlugin>();)

#include "osinfo.moc"
