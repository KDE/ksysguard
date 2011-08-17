/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    Copyright (c) 2006 John Tapsell <john.tapsell@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; version 2 of
    the License, or (at your option) version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QDomElement>
#include <QTimer>
#include <QLineEdit>
#include <QTreeView>
#include <QCheckBox>
#include <QHeaderView>
#include <QStackedLayout>

#include <kdebug.h>

#include "ProcessController.moc"
#include "ProcessController.h"
#include "processui/ksysguardprocesslist.h"
#include "processes.h"

//#define DO_MODELCHECK
#ifdef DO_MODELCHECK
#include "modeltest.h"
#endif
ProcessController::ProcessController(QWidget* parent,  SharedSettings *workSheetSettings)
    : KSGRD::SensorDisplay(parent, QString(), workSheetSettings)
{
    mProcessList = NULL;
    mProcesses = NULL;
}

void
ProcessController::sensorError(int, bool err)
{
    if (err == sensors().at(0)->isOk())
    {
        if (err)  {
            kDebug(1215) << "SensorError called with an error";
        }
        /* This happens only when the sensorOk status needs to be changed. */
        sensors().at(0)->setIsOk( !err );
    }
    setSensorOk(sensors().at(0)->isOk());
}

bool
ProcessController::restoreSettings(QDomElement& element)
{
    bool result = addSensor(element.attribute("hostName"),
                element.attribute("sensorName"),
                (element.attribute("sensorType").isEmpty() ? "table" : element.attribute("sensorType")),
                QString());
    if(!result) return false;

    int version = element.attribute("version", "0").toUInt();
    if(version == PROCESSHEADERVERSION) {  //If the header has changed, the old settings are no longer valid.  Only restore if version is the same
        mProcessList->restoreHeaderState(QByteArray::fromBase64(element.attribute("treeViewHeader").toLatin1()));
    }

    bool showTotals = element.attribute("showTotals", "1").toUInt();
    mProcessList->setShowTotals(showTotals);


    int units = element.attribute("units", QString::number((int)ProcessModel::UnitsKB)).toUInt();
    mProcessList->setUnits((ProcessModel::Units)units);

    int ioUnits = element.attribute("ioUnits", QString::number((int)ProcessModel::UnitsKB)).toUInt();
    mProcessList->processModel()->setIoUnits((ProcessModel::Units)ioUnits);

    int ioInformation = element.attribute("ioInformation", QString::number((int)ProcessModel::ActualBytesRate)).toUInt();
    mProcessList->processModel()->setIoInformation((ProcessModel::IoInformation)ioInformation);

    bool showCommandLineOptions = element.attribute("showCommandLineOptions", "0").toUInt();
    mProcessList->processModel()->setShowCommandLineOptions(showCommandLineOptions);

    bool showTooltips = element.attribute("showTooltips", "1").toUInt();
    mProcessList->processModel()->setShowingTooltips(showTooltips);

    bool normalizeCPUUsage = element.attribute("normalizeCPUUsage", "1").toUInt();
    mProcessList->processModel()->setNormalizedCPUUsage(normalizeCPUUsage);

    int filterState = element.attribute("filterState", QString::number((int)ProcessFilter::AllProcesses)).toUInt();
    mProcessList->setState((ProcessFilter::State)filterState);

    SensorDisplay::restoreSettings(element);
    return result;
}

bool ProcessController::saveSettings(QDomDocument& doc, QDomElement& element)
{
    if(!mProcessList)
        return false;
    element.setAttribute("hostName", sensors().at(0)->hostName());
    element.setAttribute("sensorName", sensors().at(0)->name());
    element.setAttribute("sensorType", sensors().at(0)->type());

    element.setAttribute("version", QString::number(PROCESSHEADERVERSION));
    element.setAttribute("treeViewHeader", QString::fromLatin1(mProcessList->treeView()->header()->saveState().toBase64()));
    element.setAttribute("showTotals", mProcessList->showTotals()?1:0);

    element.setAttribute("units", (int)(mProcessList->units()));
    element.setAttribute("ioUnits", (int)(mProcessList->processModel()->ioUnits()));
    element.setAttribute("ioInformation", (int)(mProcessList->processModel()->ioInformation()));
    element.setAttribute("showCommandLineOptions", mProcessList->processModel()->isShowCommandLineOptions());
    element.setAttribute("showTooltips", mProcessList->processModel()->isShowingTooltips());
    element.setAttribute("normalizeCPUUsage", mProcessList->processModel()->isNormalizedCPUUsage());
    element.setAttribute("filterState", (int)(mProcessList->state()));

    SensorDisplay::saveSettings(doc, element);

    return true;
}

void ProcessController::timerTick()  {
    mProcessList->updateList();

}
void ProcessController::answerReceived( int id, const QList<QByteArray>& answer ) {
    if(mProcesses)
        mProcesses->answerReceived(id, answer);
}

bool ProcessController::addSensor(const QString& hostName,
                                 const QString& sensorName,
                                 const QString& sensorType,
                                 const QString& title)
{
    if (sensorType != "table")
        return false;


    QStackedLayout *layout = new QStackedLayout(this);
    mProcessList = new KSysGuardProcessList(this, hostName);
    mProcessList->setUpdateIntervalMSecs(0); //we will call updateList() manually
    mProcessList->setContentsMargins(0,0,0,0);
    mProcessList->setScriptingEnabled(true);
    addActions(mProcessList->actions());
    connect(mProcessList, SIGNAL(updated()), this, SIGNAL(updated()));
    connect(mProcessList, SIGNAL(processListChanged()), this, SIGNAL(processListChanged()));
    mProcessList->setContextMenuPolicy( Qt::CustomContextMenu );
    connect(mProcessList, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));

    layout->addWidget(mProcessList);

    /** To use a remote sensor, we need to drill down through the layers, to connect to the remote processes.  Then connect to its signals and slots.
     *  It's horrible I know :( */
    if(!hostName.isEmpty() && hostName != "localhost") {
        KSysGuard::Processes *processes = mProcessList->processModel()->processController();
        mProcesses = processes;
        if(processes) {
            connect( processes, SIGNAL(runCommand(QString,int)), SLOT(runCommand(QString,int)));
        }

    }

    setPlotterWidget(mProcessList);

    QTimer::singleShot(0, mProcessList->filterLineEdit(), SLOT(setFocus()));

    registerSensor(new KSGRD::SensorProperties(hostName, sensorName, sensorType, title));
    /* This just triggers the first communication. The full set of
    * requests are send whenever the sensor reconnects (detected in
    * sensorError(). */
    sensors().at(0)->setIsOk(true); //Assume it is okay from the start
    setSensorOk(sensors().at(0)->isOk());
    emit processListChanged();
    return true;
}

void ProcessController::runCommand(const QString &command, int id) {
    sendRequest(sensors().at(0)->hostName(), command, id);
}

