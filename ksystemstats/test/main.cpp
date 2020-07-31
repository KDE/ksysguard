/********************************************************************
Copyright 2020 David Edmundson <davidedmundson@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include <QCoreApplication>
#include <QDebug>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QDBusMetaType>

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <iostream>

#include <ksysguard/formatter/Formatter.h>


#include "kstatsiface.h"

class SensorWatcher : public QCoreApplication
{
    Q_OBJECT

public:
    SensorWatcher(int &argc, char **argv);
    ~SensorWatcher() = default;

    void subscribe(const QStringList &sensorNames);
    void list();
    void showDetails(const QStringList &sensorNames);

    void setShowDetails(bool details);

private:
    void onNewSensorData(const SensorDataList &changes);
    void onSensorMetaDataChanged(const SensorInfoMap &sensors);
    void showSensorDetails(const SensorInfoMap &sensors);
    OrgKdeKSysGuardDaemonInterface *m_iface;
    bool m_showDetails = false;
};

int main(int argc, char **argv)
{
    qDBusRegisterMetaType<SensorData>();
    qDBusRegisterMetaType<SensorInfo>();
    qDBusRegisterMetaType<SensorDataList>();
    qDBusRegisterMetaType<QHash<QString, SensorInfo>>();
    qDBusRegisterMetaType<QStringList>();

    SensorWatcher app(argc, argv);

    QCommandLineParser parser;
    auto listSensorsOption = QCommandLineOption(QStringLiteral("list"), QStringLiteral("List Available Sensors"));
    parser.addOption(listSensorsOption);

    parser.addOption({ QStringLiteral("details"), QStringLiteral("Show detailed information about selected sensors") });

    parser.addPositionalArgument(QStringLiteral("sensorNames"), QStringLiteral("List of sensors to monitor"), QStringLiteral("sensorId1 sensorId2  ..."));
    parser.addHelpOption();
    parser.process(app);

    if (parser.isSet(listSensorsOption)) {
        app.list();
    } else if (parser.positionalArguments().isEmpty()) {
        qDebug() << "No sensors specified.";
        parser.showHelp(-1);
    } else {
        app.setShowDetails(parser.isSet(QStringLiteral("details")));
        app.subscribe(parser.positionalArguments());
        app.exec();
    }
}

SensorWatcher::SensorWatcher(int &argc, char **argv)
    : QCoreApplication(argc, argv)
    , m_iface(new OrgKdeKSysGuardDaemonInterface("org.kde.ksystemstats",
          "/",
          QDBusConnection::sessionBus(),
          this))
{
    connect(m_iface, &OrgKdeKSysGuardDaemonInterface::newSensorData, this, &SensorWatcher::onNewSensorData);
    connect(m_iface, &OrgKdeKSysGuardDaemonInterface::sensorMetaDataChanged, this, &SensorWatcher::onSensorMetaDataChanged);
}

void SensorWatcher::subscribe(const QStringList &sensorNames)
{
    m_iface->subscribe(sensorNames);

    auto pendingInitialValues = m_iface->sensorData(sensorNames);
    pendingInitialValues.waitForFinished();
    onNewSensorData(pendingInitialValues.value());

    if (m_showDetails) {
        auto pendingSensors = m_iface->sensors(sensorNames);
        pendingSensors.waitForFinished();

        auto sensors = pendingSensors.value();
        showSensorDetails(sensors);
    }
}

void SensorWatcher::onNewSensorData(const SensorDataList &changes)
{
    for (const auto &entry : changes) {
        std::cout << qPrintable(entry.sensorProperty) << ' ' << qPrintable(entry.payload.toString()) << std::endl;
    }
}

void SensorWatcher::onSensorMetaDataChanged(const SensorInfoMap &sensors)
{
    if (!m_showDetails) {
        return;
    }

    std::cout << "Sensor metadata changed\n";
    showSensorDetails(sensors);
}

void SensorWatcher::list()
{
    auto pendingSensors = m_iface->allSensors();
    pendingSensors.waitForFinished();
    auto sensors = pendingSensors.value();
    for (auto it = sensors.constBegin(); it != sensors.constEnd(); it++) {
        std::cout << qPrintable(it.key()) << ' ' << qPrintable(it.value().name) << std::endl;
    }
}

void SensorWatcher::setShowDetails(bool details)
{
    m_showDetails = details;
}

void SensorWatcher::showSensorDetails(const SensorInfoMap &sensors)
{
    for (auto it = sensors.constBegin(); it != sensors.constEnd(); ++it) {
        auto info = it.value();
        std::cout << qPrintable(it.key()) << "\n";
        std::cout << "    Name:        " << qPrintable(info.name) << "\n";
        std::cout << "    Short Name:  " << qPrintable(info.shortName) << "\n";
        std::cout << "    Description: " << qPrintable(info.description) << "\n";
        std::cout << "    Unit:        " << qPrintable(KSysGuard::Formatter::symbol(info.unit)) << "\n";
        std::cout << "    Minimum:     " << info.min << "\n";
        std::cout << "    Maximum:     " << info.max << "\n";
    }
}

#include "main.moc"
