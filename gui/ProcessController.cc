/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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

	// Create the box that will contain the other widgets.
	box = new QGroupBox(this, "pList_box"); 
	CHECK_PTR(box);

	/* All RMB clicks to the box frame will be handled by 
	 * SensorDisplay::eventFilter. */
	box->installEventFilter(this);

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
	// TODO: we need to check first if the backend supports the 'kill' command.
	bKill = new QPushButton(i18n("&Kill"), this, "bKill");
	CHECK_PTR(bKill);
	bKill->setMinimumSize(bKill->sizeHint());
	connect(bKill, SIGNAL(clicked()), this, SLOT(killProcess()));

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
	box->setGeometry(0, 0, width(), height());

    QWidget::resizeEvent(ev);
}

bool
ProcessController::addSensor(const QString& hostName,
							 const QString& sensorName,
							 const QString& title)
{
	registerSensor(hostName, sensorName, title);
	sendRequest(hostName, "ps?", 1);

	if (title.isEmpty())
		box->setTitle(QString(i18n("%1: Running Processes")).arg(hostName));
	else
		box->setTitle(title);

	return (true);
}

void
ProcessController::updateList()
{
	sendRequest(*hostNames.at(0), "ps", 2);
}

void
ProcessController::killProcess(int pid, int sig)
{
	sendRequest(*hostNames.at(0), QString("kill %1 %2" ).arg(pid).arg(sig), 3);
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
		if (KMessageBox::warningYesNo(this,
									  i18n("Do you want to kill the\n"
										   "selected processes?")) ==
			KMessageBox::No)
		{
			return;
		}
	}

	// send kill signal to all seleted processes
	QValueListConstIterator<int> it;
	for (it = selectedPIds.begin(); it != selectedPIds.end(); ++it)
		sendRequest(*hostNames.at(0), QString("kill %1 %2" ).arg(*it)
					.arg(MENU_ID_SIGKILL), 3);
	updateList();
}

void
ProcessController::answerReceived(int id, const QString& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(false);

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
			sensorError(true);
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
			sensorError(true);
			SensorMgr->resynchronize(*hostNames.at(0));
		}
		break;
	case 3:
		// result of kill operation, we currently don't care about it.
		break;
	}
}

void
ProcessController::sensorError(bool err)
{
	if (err == sensorOk)
	{
		/* This happens only when the sensorOk status needs to be changed. */
		sensorOk = !err;
	}
	pList->setSensorOk(sensorOk);
}

bool
ProcessController::load(QDomElement& el)
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
ProcessController::save(QDomDocument& doc, QDomElement& display)
{
	display.setAttribute("hostName", *hostNames.at(0));
	display.setAttribute("sensorName", *sensorNames.at(0));
	display.setAttribute("tree", (uint) xbTreeView->isChecked());
	display.setAttribute("pause", (uint) xbPause->isChecked());
	display.setAttribute("filter", cbFilter->currentItem());
	display.setAttribute("sortColumn", pList->getSortColumn());
	display.setAttribute("incrOrder", pList->getIncreasing());

	if (!pList->save(doc, display))
		return (false);

	modified = false;

	return (true);
}
