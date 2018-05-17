/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    Copyright (c) 2006 John Tapsell <john.tapsell@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy 
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef _ProcessController_h_
#define _ProcessController_h_

#include <QList>
#include <QWidget>

#include <SensorDisplay.h>

class KSysGuardProcessList;
namespace KSysGuard {
class Processes;
}
/**
 * This widget implements a process list page. Besides the process
 * list which is implemented as a ProcessList, it contains two
 * comboxes and two buttons.  The combo boxes are used to set the
 * update rate and the process filter.  The buttons are used to force
 * an immediate update and to kill a process.
 */
class ProcessController : public KSGRD::SensorDisplay
{
    Q_OBJECT

public:
    ProcessController(QWidget* parent,  SharedSettings *workSheetSettings);
    ~ProcessController() Q_DECL_OVERRIDE { }

    /* Functions for SensorDisplay*/

    bool restoreSettings(QDomElement& element) Q_DECL_OVERRIDE;

    bool saveSettings(QDomDocument& doc, QDomElement& element) Q_DECL_OVERRIDE;

    void timerTick() Q_DECL_OVERRIDE;

    bool addSensor(const QString&, const QString&, const QString&, const QString&) Q_DECL_OVERRIDE;

    void sensorError(int, bool err) Q_DECL_OVERRIDE;

    void configureSettings() Q_DECL_OVERRIDE { }

    bool hasSettingsDialog() const Q_DECL_OVERRIDE
    {
        return false;
    }

    void answerReceived(int id, const QList<QByteArray>& answer ) Q_DECL_OVERRIDE;

    KSysGuardProcessList* processList()
    {
        return mProcessList;
    }

Q_SIGNALS:
    void updated();
    void processListChanged();
private Q_SLOTS:
    void runCommand(const QString &command, int id);

private:
    KSysGuardProcessList *mProcessList;
    KSysGuard::Processes *mProcesses;
};

#endif
