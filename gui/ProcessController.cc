/*
    KTop, the KDE Task Manager and System Monitor
   
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <assert.h>

#include <qtextstream.h>

#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "SignalIDs.h"
#include "ksysguard.h"
#include "SensorManager.h"
#include "ProcessController.moc"

ProcessController::ProcessController(QWidget* parent, const char* name)
	: SensorDisplay(parent, name)
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

	/* All RMB clicks to the box frame will be handled by 
	 * SensorDisplay::eventFilter. */
	frame->installEventFilter(this);

	// Create the table that lists the processes.
	pList = new ProcessList(this, "pList");    
	CHECK_PTR(pList);
	connect(pList, SIGNAL(killProcess(int, int)),
			this, SLOT(killProcess(int, int)));

	/* All RMB clicks to the plist widget will be handled by 
	 * SensorDisplay::eventFilter. */
	pList->installEventFilter(this);
	
	// Create the check box to switch between tree view and list view.
	xbTreeView = new QCheckBox(i18n("&Tree"), this, "xbTreeView");
	CHECK_PTR(xbTreeView);
	xbTreeView->setMinimumSize(xbTreeView->sizeHint());
	connect(xbTreeView, SIGNAL(toggled(bool)),
			this, SLOT(setTreeView(bool)));

	/* Create the combo box to configure the process filter. The
	 * cbFilter must be created prior to constructing pList as the
	 * pList constructor sets cbFilter to its start value. */
	cbFilter = new QComboBox(this, "pList_cbFilter");
	CHECK_PTR(cbFilter);
	cbFilter->insertItem(i18n("All processes"), 0);
	cbFilter->insertItem(i18n("System processes"), 1);
	cbFilter->insertItem(i18n("User processes"), 2);
	cbFilter->insertItem(i18n("Own processes"), 3);
	cbFilter->setMinimumSize(cbFilter->sizeHint());

	/* When the both cbFilter and pList are constructed we can connect the
	 * missing link. */
	connect(cbFilter, SIGNAL(activated(int)),
			pList, SLOT(setFilterMode(int)));

	// Create the check box to pause the automatic list update.
	xbPause = new QCheckBox(i18n("&Pause"), this, "xbPause");
	CHECK_PTR(xbPause);
	xbPause->setMinimumSize(xbPause->sizeHint());
	connect(xbPause, SIGNAL(toggled(bool)), this, SLOT(togglePause(bool)));

	// Create the 'Refresh' button.
	bRefresh = new QPushButton(i18n("&Refresh"), this, "bRefresh");
	CHECK_PTR(bRefresh);
	bRefresh->setMinimumSize(bRefresh->sizeHint());
	connect(bRefresh, SIGNAL(clicked()), this, SLOT(updateList()));

	// Create the 'Kill' button.
	bKill = new QPushButton(i18n("&Kill"), this, "bKill");
	CHECK_PTR(bKill);
	bKill->setMinimumSize(bKill->sizeHint());
	connect(bKill, SIGNAL(clicked()), this, SLOT(killProcess()));
	/* Disable the kill button until we know that the daemon supports the
	 * kill command. */
	bKill->setEnabled(false);
	killSupported = false;

	// Setup the geometry management.
	gm = new QVBoxLayout(this, 10);
	CHECK_PTR(gm);
	gm->addSpacing(15);
	gm->addWidget(pList, 1);

	gm1 = new QHBoxLayout();
	CHECK_PTR(gm1);
	gm->addLayout(gm1, 0);
	gm1->addStretch();
	gm1->addWidget(xbTreeView);
	gm1->addStretch();
	gm1->addWidget(cbFilter);
	gm1->addStretch();
	gm1->addWidget(xbPause);
	gm1->addStretch();
	gm1->addWidget(bRefresh);
	gm1->addStretch();
	gm1->addWidget(bKill);
	gm1->addStretch();
	gm->addSpacing(5);

	gm->activate();

	setMinimumSize(sizeHint());

	modified = false;
}

void
ProcessController::resizeEvent(QResizeEvent* ev)
{
	frame->setGeometry(0, 0, width(), height());

    QWidget::resizeEvent(ev);
}

bool
ProcessController::addSensor(const QString& hostName,
							 const QString& sensorName,
							 const QString& title)
{
	registerSensor(hostName, sensorName, title);
	sendRequest(hostName, "ps?", 1);
	sendRequest(hostName, "test kill", 4);

	if (title.isEmpty())
		frame->setTitle(QString(i18n("%1: Running Processes")).arg(hostName));
	else
		frame->setTitle(title);

	return (true);
}

void
ProcessController::updateList()
{
	sendRequest(sensors.at(0)->hostName, "ps", 2);
}

void
ProcessController::killProcess(int pid, int sig)
{
	sendRequest(sensors.at(0)->hostName,
				QString("kill %1 %2" ).arg(pid).arg(sig), 3);
	updateList();
}

void
ProcessController::killProcess()
{
	const QValueList<int>& selectedPIds = pList->getSelectedPIds();

	if (selectedPIds.isEmpty())
	{
		KMessageBox::sorry(this,
						   i18n("You need to select a process first!"));
		return;
	}
	else
	{
		if (KMessageBox::warningYesNo(
			this, QString(i18n("Do you want to kill the\n"
							   "selected %1 process(es)?"))
			.arg(selectedPIds.count())) ==
			KMessageBox::No)
		{
			return;
		}
	}

	// send kill signal to all seleted processes
	QValueListConstIterator<int> it;
	for (it = selectedPIds.begin(); it != selectedPIds.end(); ++it)
		sendRequest(sensors.at(0)->hostName, QString("kill %1 %2" ).arg(*it)
					.arg(MENU_ID_SIGKILL), 3);
	updateList();
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
		SensorTokenizer lines(answer, '\n');
		if (lines.numberOfTokens() != 2)
		{
			kdDebug () << "ProcessController::answerReceived(1)"
				  "wrong number of lines [" <<  answer << "]" << endl;
			sensorError(id, true);
			return;
		}
		SensorTokenizer headers(lines[0], '\t');
		SensorTokenizer colTypes(lines[1], '\t');
		
		pList->removeColumns();
		for (unsigned int i = 0; i < headers.numberOfTokens(); i++)
		{
			QString header;
			if (dict[headers[i]])
				header = *dict[headers[i]];
			else
				header = headers[i];
			pList->addColumn(header, colTypes[i]);
		}

		timerOn();

		break;
	}
	case 2:
		/* We have received the answer to a ps command that contains a
		 * list of processes with various additional information. Sometimes,
		 * for yet unknown reason the connection gets distorted and the
		 * information is corrupted. As a workaround we simply restart the
		 * connection. The corruption seems to affect only the ps output. */
		if (!pList->update(answer))
		{
			sensorError(id, true);
			SensorMgr->resynchronize(sensors.at(0)->hostName);
		}
		break;
	case 3:
	{
		// result of kill operation
		kdDebug() << answer << endl;
		SensorTokenizer vals(answer, '\t');
		switch (vals[0].toInt())
		{
		case 0:	// successfull kill operation
			break;
		case 1:	// unknown error
			SensorMgr->notify(
				QString(i18n("Error during kill of process %1!"))
				.arg(vals[1]));
			break;
		case 2:
			SensorMgr->notify(
				QString(i18n("Insufficient permissions to kill "
							 "process %1!")).arg(vals[1]));
			break;
		case 3:
			SensorMgr->notify(
				QString(i18n("Process %1 disappeared already!")).arg(vals[1]));
			break;
		case 4:
			SensorMgr->notify(i18n("Invalid Signal!"));
			break;
		}
		break;
	}
	case 4:
		killSupported = (answer.toInt() == 1);
		pList->setKillSupported(killSupported);
		bKill->setEnabled(killSupported);
		break;
	}
}

void
ProcessController::sensorError(int, bool err)
{
	if (err == sensors.at(0)->ok)
	{
		/* This happens only when the sensorOk status needs to be changed. */
		sensors.at(0)->ok = !err;
	}
	pList->setSensorOk(sensors.at(0)->ok);
}

bool
ProcessController::createFromDOM(QDomElement& el)
{
	bool result = addSensor(el.attribute("hostName"),
							el.attribute("sensorName"),
							QString::null);

	xbTreeView->setChecked(el.attribute("tree").toInt());
	setTreeView(el.attribute("tree").toInt());
	xbPause->setChecked(el.attribute("pause").toInt());
	togglePause(el.attribute("pause").toInt());

	uint filter = el.attribute("filter").toUInt();
	cbFilter->setCurrentItem(filter);
	filterModeChanged(filter);

	uint col = el.attribute("sortColumn").toUInt();
	bool inc = el.attribute("incrOrder").toUInt();

	if (!pList->load(el))
		return (false);

	pList->setSortColumn(col, inc);

	modified = false;

	return (result);
}

bool
ProcessController::addToDOM(QDomDocument& doc, QDomElement& display, bool save)
{
	display.setAttribute("hostName", sensors.at(0)->hostName);
	display.setAttribute("sensorName", sensors.at(0)->name);
	display.setAttribute("tree", (uint) xbTreeView->isChecked());
	display.setAttribute("pause", (uint) xbPause->isChecked());
	display.setAttribute("filter", cbFilter->currentItem());
	display.setAttribute("sortColumn", pList->getSortColumn());
	display.setAttribute("incrOrder", pList->getIncreasing());

	if (!pList->save(doc, display))
		return (false);

	if (save)
		modified = false;

	return (true);
}
