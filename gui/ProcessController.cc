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

#include "ktop.h"
#include "SensorManager.h"
#include "ProcessController.moc"

ProcessController::ProcessController(QWidget* parent, const char* name)
	: SensorDisplay(parent, name)
{
	// Create the box that will contain the other widgets.
	box = new QGroupBox(this, "pList_box"); 
	CHECK_PTR(box);

	// Create the table that lists the processes.
	pList = new ProcessList(this, "pList");    
	CHECK_PTR(pList);

	// Create the check box to switch between tree view and list view.
	xbTreeView = new QCheckBox("Tree", this, "xbTreeView");
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
	xbPause = new QCheckBox("Pause", this, "xbPause");
	CHECK_PTR(xbPause);
	xbPause->setMinimumSize(xbPause->sizeHint());
	connect(xbPause, SIGNAL(toggled(bool)), this, SLOT(togglePause(bool)));

	// Create the 'Refresh' button.
	bRefresh = new QPushButton(i18n("Refresh"), this, "bRefresh");
	CHECK_PTR(bRefresh);
	bRefresh->setMinimumSize(bRefresh->sizeHint());
	connect(bRefresh, SIGNAL(clicked()), this, SLOT(updateList()));

	// Create the 'Kill' button.
	bKill = new QPushButton(i18n("Kill"), this, "bKill");
	CHECK_PTR(bKill);
	bKill->setMinimumSize(bKill->sizeHint());
	connect(bKill,SIGNAL(clicked()), this, SLOT(killProcess()));

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

	modified = FALSE;
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
	registerSensor(hostName, sensorName);
	sendRequest(hostName, "ps?", 1);

	if (title.isEmpty())
		box->setTitle(QString(i18n("%1: Running Processes")).arg(hostName));
	else
		box->setTitle(title);

	return (TRUE);
}

void
ProcessController::updateList()
{
	sendRequest(*hostNames.at(0), "ps", 2);
}

void
ProcessController::killProcess(void)
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
		sendRequest(*hostNames.at(0), QString("kill %1 9" ).arg(*it), 3);
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
			debug(QString("ProcessController::answerReceived(1)"
				  "wrong number of lines [%1]").arg(answer));
			return;
		}
		SensorTokenizer headers(lines[0], '\t');
		SensorTokenizer colTypes(lines[1], '\t');
		
		pList->removeColumns();
		for (unsigned int i = 0; i < headers.numberOfTokens(); i++)
			pList->addColumn(headers[i], colTypes[i]);

		break;
	}
	case 2:
		/* We have received the answer to a ps command that contains a
		 * list of processes with various additional information. */
		pList->update(answer);
		break;
	case 3:
		// result of kill operation, we currently don't care about it.
		break;
	}
}

bool
ProcessController::load(QDomElement& el)
{
	bool result = addSensor(el.attribute("hostName"),
							el.attribute("sensorName"),
							QString::null);

	if (el.attribute("tree") == "on")
	{
		xbTreeView->setChecked(TRUE);
		setTreeView(TRUE);
	}
	if (el.attribute("pause") == "on")
	{
		xbPause->setChecked(TRUE);
		togglePause(TRUE);
	}

	int filter = el.attribute("filter").toUInt();
	cbFilter->setCurrentItem(filter);
	filterModeChanged(filter);

	return (result);
}

bool
ProcessController::save(QTextStream& s)
{
	s << "hostName=\"" << *hostNames.at(0) << "\" "
	  << "sensorName=\"" << *sensorNames.at(0) << "\" "
	  << "tree=\"";
	if (xbTreeView->isChecked())
		s << "on";
	else
		s << "off";
	s << "\" ";

	s << "pause=\"";
	if (xbPause->isChecked())
		s << "on";
	else
		s << "off";
	s << "\" ";
	s << "filter=\"" << cbFilter->currentItem() << "\">\n";

	modified = FALSE;

	return (TRUE);
}
