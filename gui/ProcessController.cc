/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

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

#include <kapp.h>
#include <klocale.h>

#include "ktop.h"
#include "ProcessController.moc"

#define NONE -1

ProcessController::ProcessController(QWidget* parent, const char* name)
	: SensorDisplay(parent, name)
{
	// Create the box that will contain the other widgets.
	box = new QGroupBox(this, "pList_box"); 
	box->setTitle(i18n("Running Processes"));
	CHECK_PTR(box);

	// Create the table that lists the processes.
	pList = new ProcessList(this, "pList");    
	CHECK_PTR(pList);

	// no timer started yet
	timerId = NONE;
	refreshRate = REFRESH_MEDIUM;

	treeViewCB = new QCheckBox("Show Tree", this, "TreeViewCB");
	CHECK_PTR(treeViewCB);
	treeViewCB->setMinimumSize(treeViewCB->sizeHint());
	connect(treeViewCB, SIGNAL(toggled(bool)),
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

	// Create the 'Refresh Now' button.
	bRefresh = new QPushButton(i18n("Refresh Now"), this, "pList_bRefresh");
	CHECK_PTR(bRefresh);
	bRefresh->setMinimumSize(bRefresh->sizeHint());
	connect(bRefresh, SIGNAL(clicked()), pList, SLOT(update()));

	// Create the 'Kill task' button.
	bKill = new QPushButton(i18n("Kill task"), this, "pList_bKill");
	CHECK_PTR(bKill);
	bKill->setMinimumSize(bKill->sizeHint());
	connect(bKill,SIGNAL(clicked()), pList, SLOT(killProcess()));

	// Setup the geometry management.
	gm = new QVBoxLayout(this, 10);
	gm->addSpacing(15);
	gm->addWidget(pList, 1);

	gm1 = new QHBoxLayout();
	gm->addLayout(gm1, 0);
	gm1->addStretch();
	gm1->addWidget(treeViewCB);
	gm1->addStretch();
	gm1->addWidget(cbFilter);
	gm1->addStretch();
	gm1->addWidget(bRefresh);
	gm1->addStretch();
	gm1->addWidget(bKill);
	gm1->addStretch();
	gm->addSpacing(5);

	gm->activate();

	setMinimumSize(sizeHint());

	pList->loadSettings();
}

void
ProcessController::resizeEvent(QResizeEvent* ev)
{
	box->setGeometry(5, 5, width() - 10, height() - 10);

    QWidget::resizeEvent(ev);
}

bool
ProcessController::addSensor(SensorAgent* sa, const QString& /* sensorName */,
							 const QString& /* title */)
{
	sensorAgent = sa;
	sa->sendRequest("ps?", (SensorClient*) this, 1);

	return (TRUE);
}

void 
ProcessController::setRefreshRate(int r)
{
	assert(r >= REFRESH_MANUAL && r <= REFRESH_FAST);

	timerOff();
	switch (refreshRate = r)
	{
	case REFRESH_MANUAL:
		break;

	case REFRESH_SLOW:
		timerInterval = 20000;
		break;

	case REFRESH_MEDIUM:
		timerInterval = 7000;
		break;

	case REFRESH_FAST:
	default:
		timerInterval = 1000;
		break;
	}

	// only re-start the timer if auto mode is enabled
	if (refreshRate != REFRESH_MANUAL)
		timerOn();

//	emit(refreshRateChanged(refreshRate));
}

int
ProcessController::setAutoUpdateMode(bool mode)
{
	/*
	 * If auto mode is enabled the display is updated regurlarly triggered
	 * by a timer event.
	 */

	// save current setting of the timer
	int oldmode = (timerId != NONE) ? TRUE : FALSE; 

	// set new setting
	if (mode && (refreshRate != REFRESH_MANUAL))
		timerOn();
	else
		timerOff();

	// return the previous setting
	return (oldmode);
}

void
ProcessController::answerReceived(int id, const QString& answer)
{
	switch (id)
	{
	case 1:
	{
		/* We have received the answer to a ps? command that contains
		 * the information about the table headers. */
		SensorTokenizer lines(answer, '\n');
		if (lines.numberOfTokens() != 2)
		{
			debug("ProcessController::answerReceived(1)"
				  "wrong number of lines");
			return;
		}
		SensorTokenizer headers(lines[0], '\t');
		SensorTokenizer colTypes(lines[1], '\t');
		
		pList->removeColumns();
		for (unsigned int i = 0; i < headers.numberOfTokens(); i++)
			pList->addColumn(headers[i], colTypes[i]);

		// start automatic refreshing
		setRefreshRate(refreshRate);

		break;
	}
	case 2:
		/* We have received the answer to a ps command that contains a
		 * list of processes with various additional information. */

		// disable the auto-update and save current mode
		int lastmode = setAutoUpdateMode(FALSE);

		pList->update(answer);

		// restore update mode
		setAutoUpdateMode(lastmode);

		break;
	}
}
