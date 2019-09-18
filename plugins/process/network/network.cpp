/*
 * This file is part of KSysGuard.
 * Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "network.h"

#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QProcess>
#include <QStandardPaths>
#include <QFile>

#include <KLocalizedString>
#include <KPluginFactory>

#include "networklogging.h"
#include "networkconstants.h"

using namespace KSysGuard;

NetworkPlugin::NetworkPlugin(QObject *parent, const QVariantList &args)
    : ProcessDataProvider(parent, args)
{
    const auto executable = NetworkConstants::HelperLocation;
    if (!QFile::exists(executable)) {
        qCWarning(KSYSGUARD_PLUGIN_NETWORK) << "Could not find ksgrd_network_helper";
        return;
    }

    qCDebug(KSYSGUARD_PLUGIN_NETWORK) << "Network plugin loading";
    qCDebug(KSYSGUARD_PLUGIN_NETWORK) << "Found helper at" << qPrintable(executable);

    m_inboundSensor = new ProcessAttribute(QStringLiteral("netInbound"), i18n("Download Speed"), this);
    m_inboundSensor->setShortName(i18n("Download"));
    m_inboundSensor->setUnit(KSysGuard::UnitByteRate);
    m_inboundSensor->setVisibleByDefault(true);
    m_outboundSensor = new ProcessAttribute(QStringLiteral("netOutbound"), i18n("Upload Speed"), this);
    m_outboundSensor->setShortName(i18n("Upload"));
    m_outboundSensor->setUnit(KSysGuard::UnitByteRate);
    m_outboundSensor->setVisibleByDefault(true);

    addProcessAttribute(m_inboundSensor);
    addProcessAttribute(m_outboundSensor);

    m_process = new QProcess(this);
    m_process->setProgram(executable);

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus status) {
        if (exitCode != 0 || status != QProcess::NormalExit) {
            qCWarning(KSYSGUARD_PLUGIN_NETWORK) << "Helper process terminated abnormally!";
            qCWarning(KSYSGUARD_PLUGIN_NETWORK) << m_process->readAllStandardError();
        }
    });

    connect(m_process, &QProcess::readyReadStandardOutput, this, [=]() {
        while (m_process->canReadLine()) {
            const QString line = QString::fromUtf8(m_process->readLine());

            // Each line consists of: timestamp|PID|pid|IN|in_bytes|OUT|out_bytes
            const auto parts = line.splitRef(QLatin1Char('|'), QString::SkipEmptyParts);
            if (parts.size() < 7) {
                continue;
            }

            long pid = parts.at(2).toLong();

            auto timeStamp = QDateTime::currentDateTimeUtc();
            timeStamp.setTime(QTime::fromString(parts.at(0).toString(), QStringLiteral("HH:mm:ss")));

            auto bytesIn = parts.at(4).toUInt();
            auto bytesOut = parts.at(6).toUInt();

            auto process = getProcess(pid);
            if (!process) {
                return;
            }

            m_inboundSensor->setData(process, bytesIn);
            m_outboundSensor->setData(process, bytesOut);
        }
    });
}

void NetworkPlugin::handleEnabledChanged(bool enabled)
{
    if (enabled) {
        qCDebug(KSYSGUARD_PLUGIN_NETWORK) << "Network plugin enabled, starting helper";
        m_process->start();
    } else {
        qCDebug(KSYSGUARD_PLUGIN_NETWORK) << "Network plugin disabled, stopping helper";
        m_process->terminate();
    }
}

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "networkplugin.json", registerPlugin<NetworkPlugin>();)

#include "network.moc"
