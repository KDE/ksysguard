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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!

*/

#include <assert.h>

#include <qtimer.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <ksgrd/SensorManager.h>

#include "ProcessController.moc"
#include "SignalIDs.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kpushbutton.h>



ProcessController::ProcessController(QWidget* parent, const char* name)
	: KSGRD::SensorDisplay(parent, name)
{
	dict.setAutoDelete(true);
	dict.insert("Name", new QString(i18n("Name")));
	dict.insert("PID", new QString(i18n("PID")));
	dict.insert("PPID", new QString(i18n("PPID")));
	dict.insert("UID", new QString(i18n("UID")));
	dict.insert("GID", new QString(i18n("GID")));
	dict.insert("Status", new QString(i18n("Status")));
	dict.insert("User%", new QString(i18n("User%")));
	dict.insert("System%", new QString(i18n("System%")));
	dict.insert("Nice", new QString(i18n("Nice")));
	dict.insert("VmSize", new QString(i18n("VmSize")));
	dict.insert("VmRss", new QString(i18n("VmRss")));
	dict.insert("Login", new QString(i18n("Login")));
	dict.insert("Command", new QString(i18n("Command")));

	// Create the table that lists the processes.
	pList = new ProcessList(this, "pList");
	Q_CHECK_PTR(pList);
	connect(pList, SIGNAL(killProcess(int, int)),
			this, SLOT(killProcess(int, int)));
	connect(pList, SIGNAL(reniceProcess(int, int)),
			this, SLOT(reniceProcess(int, int)));
	connect(pList, SIGNAL(listModified(bool)),
			this, SLOT(setModified(bool)));

	// Create the check box to switch between tree view and list view.
	xbTreeView = new QCheckBox(i18n("&Tree"), this, "xbTreeView");
	Q_CHECK_PTR(xbTreeView);
	xbTreeView->setMinimumSize(xbTreeView->sizeHint());
	connect(xbTreeView, SIGNAL(toggled(bool)),
			this, SLOT(setTreeView(bool)));

	/* Create the combo box to configure the process filter. The
	 * cbFilter must be created prior to constructing pList as the
	 * pList constructor sets cbFilter to its start value. */
	cbFilter = new QComboBox(this, "pList_cbFilter");
	Q_CHECK_PTR(cbFilter);
	cbFilter->insertItem(i18n("All Processes"), 0);
	cbFilter->insertItem(i18n("System Processes"), 1);
	cbFilter->insertItem(i18n("User Processes"), 2);
	cbFilter->insertItem(i18n("Own Processes"), 3);
	cbFilter->setMinimumSize(cbFilter->sizeHint());

	/* When the both cbFilter and pList are constructed we can connect the
	 * missing link. */
	connect(cbFilter, SIGNAL(activated(int)),
			this, SLOT(filterModeChanged(int)));

	// Create the 'Refresh' button.
    bRefresh = new KPushButton( KGuiItem(  i18n( "&Refresh" ), "reload" ),
            this, "bRefresh" );
	Q_CHECK_PTR(bRefresh);
	bRefresh->setMinimumSize(bRefresh->sizeHint());
	connect(bRefresh, SIGNAL(clicked()), this, SLOT(updateList()));

	// Create the 'Kill' button.
	bKill = new KPushButton(i18n("&Kill"), this, "bKill");
	Q_CHECK_PTR(bKill);
	bKill->setMinimumSize(bKill->sizeHint());
	connect(bKill, SIGNAL(clicked()), this, SLOT(killProcess()));
	/* Disable the kill button until we know that the daemon supports the
	 * kill command. */
	bKill->setEnabled(false);
	killSupported = false;

	// Setup the geometry management.
	gm = new QVBoxLayout(this, 10);
	Q_CHECK_PTR(gm);
	gm->addSpacing(15);
	gm->addWidget(pList, 1);

	gm1 = new QHBoxLayout();
	Q_CHECK_PTR(gm1);
	gm->addLayout(gm1, 0);
	gm1->addStretch();
	gm1->addWidget(xbTreeView);
	gm1->addStretch();
	gm1->addWidget(cbFilter);
	gm1->addStretch();
	gm1->addWidget(bRefresh);
	gm1->addStretch();
	gm1->addWidget(bKill);
	gm1->addStretch();
	gm->addSpacing(5);

	gm->activate();

	setPlotterWidget(pList);

	setMinimumSize(sizeHint());
}

void
ProcessController::resizeEvent(QResizeEvent* ev)
{
	frame()->setGeometry(0, 0, width(), height());

    QWidget::resizeEvent(ev);
}

bool
ProcessController::addSensor(const QString& hostName,
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

void
ProcessController::killProcess(int pid, int sig)
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
	const QValueList<int>& selectedPIds = pList->getSelectedPIds();

	if (selectedPIds.isEmpty())
	{
		KMessageBox::sorry(this,
						   i18n("You need to select a process first."));
		return;
	}
	else
	{
		QString  msg = i18n("Do you want to kill the selected process?",
				"Do you want to kill the %n selected processes?",
				selectedPIds.count());
		if (KMessageBox::warningContinueCancel(this, msg, kapp->makeStdCaption(i18n("Kill Process")), i18n("&Kill")) != KMessageBox::Continue)
		{
			return;
		}
	}

	// send kill signal to all seleted processes
	QValueListConstIterator<int> it;
	for (it = selectedPIds.begin(); it != selectedPIds.end(); ++it)
		sendRequest(sensors().at(0)->hostName(), QString("kill %1 %2" ).arg(*it)
					.arg(MENU_ID_SIGKILL), 3);

	if ( !timerOn())
	    // give ksysguardd time to update its proccess list
	    QTimer::singleShot(3000, this, SLOT(updateList()));
	else
	updateList();
}

void
ProcessController::reniceProcess(int pid, int niceValue)
{
	sendRequest(sensors().at(0)->hostName(),
				QString("setpriority %1 %2" ).arg(pid).arg(niceValue), 5);
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
		KSGRD::SensorTokenizer lines(answer, '\n');
		if (lines.count() != 2)
		{
			kdDebug (1215) << "ProcessController::answerReceived(1)"
				  "wrong number of lines [" <<  answer << "]" << endl;
			sensorError(id, true);
			return;
		}
		KSGRD::SensorTokenizer headers(lines[0], '\t');
		KSGRD::SensorTokenizer colTypes(lines[1], '\t');

		pList->removeColumns();
		for (unsigned int i = 0; i < headers.count(); i++)
		{
			QString header;
			if (dict[headers[i]])
				header = *dict[headers[i]];
			else
				header = headers[i];
			pList->addColumn(header, colTypes[i]);
		}

		break;
	}
	case 2:
		/* We have received the answer to a ps command that contains a
		 * list of processes with various additional information. */
		pList->update(answer);
		break;
	case 3:
	{
		// result of kill operation
		kdDebug(1215) << answer << endl;
		KSGRD::SensorTokenizer vals(answer, '\t');
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
		pList->setKillSupported(killSupported);
		bKill->setEnabled(killSupported);
		break;
	case 5:
	{
		// result of renice operation
		kdDebug(1215) << answer << endl;
		KSGRD::SensorTokenizer vals(answer, '\t');
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
			sendRequest(sensors().at(0)->hostName(), "ps?", 1);
			sendRequest(sensors().at(0)->hostName(), "test kill", 4);
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
							element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "table" : element.attribute("sensorType")),
							QString::null);

	xbTreeView->setChecked(element.attribute("tree").toInt());
	setTreeView(element.attribute("tree").toInt());

	uint filter = element.attribute("filter").toUInt();
	cbFilter->setCurrentItem(filter);
	filterModeChanged(filter);

	uint col = element.attribute("sortColumn").toUInt();
	bool inc = element.attribute("incrOrder").toUInt();

	if (!pList->load(element))
		return (false);

	pList->setSortColumn(col, inc);

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
	element.setAttribute("tree", (uint) xbTreeView->isChecked());
	element.setAttribute("filter", cbFilter->currentItem());
	element.setAttribute("sortColumn", pList->getSortColumn());
	element.setAttribute("incrOrder", pList->getIncreasing());

	if (!pList->save(doc, element))
		return (false);

	SensorDisplay::saveSettings(doc, element);

	if (save)
		setModified(false);

	return (true);
}
