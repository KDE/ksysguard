/*  This file is part of the KDE project

    Copyright (C) 2019 David Edmundson <davidedmundson@kde.org>

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

#include "nvidia.h"

#include <QStandardPaths>
#include <QProcess>
#include <QDebug>

#include <KLocalizedString>
#include <KPluginFactory>

#include <processcore/process.h>

using namespace KSysGuard;

NvidiaPlugin::NvidiaPlugin(QObject *parent, const QVariantList &args)
    : ProcessDataProvider(parent, args)
{
    m_sniExecutablePath = QStandardPaths::findExecutable(QStringLiteral("nvidia-smi"));
    if (m_sniExecutablePath.isEmpty()) {
        return;
    }

    m_usage = new ProcessAttribute(QStringLiteral("nvidia_usage"), i18n("GPU Usage"), this);
    m_usage->setUnit(KSysGuard::UnitPercent);
    m_memory = new ProcessAttribute(QStringLiteral("nvidia_memory"), i18n("GPU Memory"), this);
    m_memory->setUnit(KSysGuard::UnitPercent);

    addProcessAttribute(m_usage);
    addProcessAttribute(m_memory);
}

void NvidiaPlugin::handleEnabledChanged(bool enabled)
{
    if (enabled) {
        if (!m_process) {
            setup();
        }
        m_process->start();
    } else {
        if (m_process) {
            m_process->terminate();
        }
    }
}

void NvidiaPlugin::setup()
{
    m_process = new QProcess(this);
    m_process->setProgram(m_sniExecutablePath);
    m_process->setArguments({QStringLiteral("pmon")});

    connect(m_process, &QProcess::readyReadStandardOutput, this, [=]() {
        while (m_process->canReadLine()) {
            const QString line = QString::fromLatin1(m_process->readLine());
            if (line.startsWith(QLatin1Char('#'))) { //comment line
                if (line != QLatin1String("# gpu        pid  type    sm   mem   enc   dec   command\n") &&
                    line != QLatin1String("# Idx          #   C/G     %     %     %     %   name\n")) {
                    //header format doesn't match what we expected, bail before we send any garbage
                    m_process->terminate();
                }
                continue;
            }
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            const auto parts = line.splitRef(QLatin1Char(' '),  QString::SkipEmptyParts);
#else
            const auto parts = line.splitRef(QLatin1Char(' '),  Qt::SkipEmptyParts);
#endif

            // format at time of writing is
            // # gpu        pid  type    sm   mem   enc   dec   command
            if (parts.count() < 5) { // we only access up to the 5th element
                continue;
            }

            long pid = parts[1].toUInt();
            int sm = parts[3].toUInt();
            int mem = parts[4].toUInt();

            KSysGuard::Process *process = getProcess(pid);
            if(!process) {
                continue; //can in race condition etc
            }
            m_usage->setData(process, sm);
            m_memory->setData(process, mem);
        }
    });
}

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "nvidia.json", registerPlugin<NvidiaPlugin>();)

#include "nvidia.moc"
