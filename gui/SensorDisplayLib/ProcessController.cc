/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms version 2 of of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!

*/

#include <assert.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QList>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QSortFilterProxyModel>
#include <QHeaderView>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdialogbase.h>

#include <ksgrd/SensorManager.h>

#include "ProcessController.moc"
#include "ProcessController.h"
#include "SignalIDs.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kpushbutton.h>


ProcessController::ProcessController(QWidget* parent, const char* name)
	: KSGRD::SensorDisplay(parent, name), mModel(parent), mFilterModel(parent)
{
	mUi.setupUi(this);
//	mFilterModel.setSourceModel(&mModel);
//	mUi.treeView->setModel(&mFilterModel);
	mUi.treeView->setModel(&mModel);
	mSetupTreeView = false;
	mUi.treeView->header()->setClickable(true);
	mUi.treeView->header()->setSortIndicatorShown(true);
	mUi.treeView->header()->setStretchLastSection(true);
	
	connect(mUi.btnRefresh, SIGNAL(clicked()), this, SLOT(updateList()));
	connect(mUi.btnKillProcess, SIGNAL(clicked()), this, SLOT(killProcess()));
	connect(mUi.txtFilter, SIGNAL(textChanged(const QString &)), &mFilterModel, SLOT(setFilterRegExp(const QString &)));
	connect(mUi.cmbFilter, SIGNAL(currentIndexChanged(int)), &mFilterModel, SLOT(setFilter(int)));
	connect(&mModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)), this, SLOT(setupTreeView()));
	connect(&mModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(expandRows(const QModelIndex &, int, int)));
	

	setPlotterWidget(this);
	setMinimumSize(sizeHint());
}
void ProcessController::expandRows( const QModelIndex & parent, int start, int end )
{
	for(int i = start; i <= end; i++) {
		mUi.treeView->expand(mModel.index(i,0, parent));
	}
}
void ProcessController::setupTreeView()
{
	mUi.treeView->resizeColumnToContents(0);
	mUi.treeView->hideColumn(1);
	mUi.treeView->hideColumn(2);
}

void ProcessController::resizeEvent(QResizeEvent* ev)
{
	frame()->setGeometry(0, 0, width(), height());

	QWidget::resizeEvent(ev);
}

bool ProcessController::addSensor(const QString& hostName,
				  const QString& sensorName,
				  const QString& sensorType,
				  const QString& title)
{
	if (sensorType != "table")
		return (false);

	registerSensor(new KSGRD::SensorProperties(hostName, sensorName, sensorType, title));
	/* This just triggers the first communication. The full set of
	 * requests are send whenever the sensor reconnects (detected in
	 * sensorError(). */

	mModel.setIsLocalhost(sensors().at(0)->isLocalhost()); //by telling our model that this is localhost, it can provide more information about the data it has
	sendRequest(hostName, "test kill", 4);

	if (title.isEmpty())
		setTitle(i18n("%1: Running Processes").arg(hostName));
	else
		setTitle(title);

	return (true);
}

void
ProcessController::updateList()
{
	sendRequest(sensors().at(0)->hostName(), "ps", 2);
}

void ProcessController::killProcess(int pid, int sig)
{
	sendRequest(sensors().at(0)->hostName(),
				QString("kill %1 %2" ).arg(pid).arg(sig), 3);

	if ( !timerOn() )
	    // give ksysguardd time to update its proccess list
	    QTimer::singleShot(3000, this, SLOT(updateList()));
	else
	    updateList();
}

void
ProcessController::killProcess()
{
//	mUi.treeView->
/*	const QStringList& selectedAsStrings = pList->getSelectedAsStrings();
	if (selectedAsStrings.isEmpty())
	{
		KMessageBox::sorry(this,
						   i18n("You need to select a process first."));
		return;
	}
	else
	{
		QString  msg = i18n("Do you want to kill the selected process?",
				"Do you want to kill the %n selected processes?",
				selectedAsStrings.count());

		KDialogBase *dlg = new KDialogBase (  i18n ("Kill Process"),
						      KDialogBase::Yes | KDialogBase::Cancel,
						      KDialogBase::Yes, KDialogBase::Cancel, this->parentWidget(),
						      "killconfirmation",
			       			      true, true, KGuiItem(i18n("Kill")));

		bool dontAgain = false;

		int res = KMessageBox::createKMessageBox(dlg, QMessageBox::Question,
			                                 msg, selectedAsStrings,
							 i18n("Do not ask again"), &dontAgain,
							 KMessageBox::Notify);

		if (res != KDialogBase::Yes)
		{
			return;
		}
	}

	const QList<int>& selectedPIds = pList->getSelectedPIds();

        for (int i = 0; i < selectedPIds.size(); ++i) {
            sendRequest(sensors().at(0)->hostName(), QString("kill %1 %2" ).arg(selectedPIds.at( i ))
                       .arg(MENU_ID_SIGKILL), 3);
        }
	if ( !timerOn())
		// give ksysguardd time to update its proccess list
		QTimer::singleShot(3000, this, SLOT(updateList()));
	else
		updateList();*/
}

void
ProcessController::reniceProcess(int pid, int niceValue)
{
	sendRequest(sensors().at(0)->hostName(),
				QString("setpriority %1 %2" ).arg(pid).arg(niceValue), 5);
	sendRequest(sensors().at(0)->hostName(), "ps", 2);  //update the display afterwards
}

void
ProcessController::answerReceived(int id, const QString& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);
	switch (id)
	{
	case 1:
	{
		/* We have received the answer to a ps? command that contains
		 * the information about the table headers. */
		QStringList lines = answer.trimmed().split('\n');
		if (lines.count() != 2)
		{
			kDebug (1215) << "ProcessController::answerReceived(1)"
				  "wrong number of lines [" <<  answer << "]" << endl;
			sensorError(id, true);
			return;
		}
		mModel.setHeader(lines.at(0).split('\t'));
		QString line = lines.at(1);
		QList<char> coltype;
		for(int i = 0; i < line.size(); i++)  //coltype is in the form "d\tf\tS\t" etc, so split into a list of char
			if(line[i] != '\t')
				coltype << line[i].toLatin1();//FIMXE: the answer should really be a QByteArray, not a string
		mModel.setColType(coltype);

		mFilterModel.setFilterKeyColumn(0);

		break;
	}
	case 2:
	{
		/* We have received the answer to a ps command that contains a
		 * list of processes with various additional information. */
		QStringList lines = answer.trimmed().split('\n');
		QList<QStringList> data;
		QStringListIterator i(lines);
		while(i.hasNext()) {
		  data << i.next().split('\t');
		}
		mModel.setData(data);
		break;
	}
	case 3:
	{
		// result of kill operation
		KSGRD::SensorTokenizer vals(answer.trimmed(), '\t');
		switch (vals[0].toInt())
		{
		case 0:	// successful kill operation
			break;
		case 1:	// unknown error
			KSGRD::SensorMgr->notify(
				i18n("Error while attempting to kill process %1.")
				.arg(vals[1]));
			break;
		case 2:
			KSGRD::SensorMgr->notify(
				i18n("Insufficient permissions to kill "
							 "process %1.").arg(vals[1]));
			break;
		case 3:
			KSGRD::SensorMgr->notify(
				i18n("Process %1 has already disappeared.")
				.arg(vals[1]));
			break;
		case 4:
			KSGRD::SensorMgr->notify(i18n("Invalid Signal."));
			break;
		}
		break;
	}
	case 4:
		killSupported = (answer.toInt() == 1);
		//pList->setKillSupported(killSupported);
		//mUi.btnKillProcess->setEnabled(killSupported);
		break;
	case 5:
	{
		// result of renice operation
		kDebug(1215) << answer << endl;
		KSGRD::SensorTokenizer vals(answer.trimmed(), '\t');
		switch (vals[0].toInt())
		{
		case 0:	// successful renice operation
			break;
		case 1:	// unknown error
			KSGRD::SensorMgr->notify(
				i18n("Error while attempting to renice process %1.")
				.arg(vals[1]));
			break;
		case 2:
			KSGRD::SensorMgr->notify(
				i18n("Insufficient permissions to renice "
							 "process %1.").arg(vals[1]));
			break;
		case 3:
			KSGRD::SensorMgr->notify(
				i18n("Process %1 has already disappeared.")
				.arg(vals[1]));
			break;
		case 4:
			KSGRD::SensorMgr->notify(i18n("Invalid argument."));
			break;
		}
		break;
	}
	}
}

void
ProcessController::sensorError(int, bool err)
{
	if (err == sensors().at(0)->isOk())
	{
		if (!err)
		{
			/* Whenever the communication with the sensor has been
			 * (re-)established we need to requests the full set of
			 * properties again, since the back-end might be a new
			 * one. */
			sendRequest(sensors().at(0)->hostName(), "test kill", 4);
			sendRequest(sensors().at(0)->hostName(), "ps?", 1);
			sendRequest(sensors().at(0)->hostName(), "ps", 2);
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
//	mUi.chkTreeView->setChecked(element.attribute("tree").toInt());
//	setTreeView(element.attribute("tree").toInt());

	uint filter = element.attribute("filter").toUInt();
	mUi.cmbFilter->setCurrentItem(filter);

	uint col = element.attribute("sortColumn").toUInt();
	bool inc = element.attribute("incrOrder").toUInt();

//	if (!pList->load(element))
//		return (false);

	mFilterModel.sort(col,(inc)?Qt::AscendingOrder:Qt::DescendingOrder);
	
	SensorDisplay::restoreSettings(element);
	setModified(false);

	return (result);
}

bool
ProcessController::saveSettings(QDomDocument& doc, QDomElement& element, bool save)
{
	element.setAttribute("hostName", sensors().at(0)->hostName());
	element.setAttribute("sensorName", sensors().at(0)->name());
	element.setAttribute("sensorType", sensors().at(0)->type());
//	element.setAttribute("tree", (uint) mUi.chkTreeView->isChecked());
	element.setAttribute("filter", mUi.cmbFilter->currentItem());
//FIXME There is currently no way to get this information from qt!!
//	element.setAttribute("sortColumn", pList->getSortColumn());
//	element.setAttribute("incrOrder", pList->getIncreasing());

//	if (!pList->save(doc, element))
//		return (false);

	SensorDisplay::saveSettings(doc, element);

	if (save)
		setModified(false);

	return (true);
}

