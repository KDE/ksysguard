/*
    KTop, the KDE Task Manager
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
	Copyright (c) 1999 Chris Schlaeger
	                   cs@kde.org
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!
*/

// $Id$

#include <assert.h>

#include <ktmainwindow.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "SensorManager.h"
#include "SensorAgent.h"
#include "OSStatus.h"
#include "ktop.moc"

#define KTOP_MIN_W	50
#define KTOP_MIN_H	50

/*
 * Global variables
 */
KApplication* Kapp;

/*
 * This is the constructor for the main widget. It sets up the menu and the
 * TaskMan widget.
 */
TopLevel::TopLevel(const char *name, int sfolder)
	: KTMainWindow(name)
{
	taskman = 0;

	assert(Kapp);
	setCaption(i18n("KDE Task Manager"));

	/*
	 * create main menu
	 */
	menubar = new MainMenu(this, "MainMenu");
	connect(menubar, SIGNAL(quit()), this, SLOT(quitSlot()));
	// register the menu bar with KTMainWindow
	setMenu(menubar);

	// create the tab dialog
	taskman = new TaskMan(this, "TaskMan", sfolder);

	// register the tab dialog with KTMainWindow as client widget
	setView(taskman);

	connect(taskman, SIGNAL(enableRefreshMenu(bool)),
			menubar, SLOT(enableRefreshMenu(bool)));

	statusbar = new KStatusBar(this, "statusbar");
	statusbar->insertItem(i18n("88888 Processes"), 0);
	statusbar->insertItem(i18n("Memory: 8888888 kB used, "
							   "8888888 kB free"), 1);
	statusbar->insertItem(i18n("Swap: 8888888 kB used, "
							   "8888888 kB free"), 2);
	setStatusBar(statusbar);

	localhost = SensorMgr->engage("localhost");
	/* Request info about the swapspace size and the units it is measured in.
	 * The requested info will be received by answerReceived(). */
	localhost->sendRequest("memswap?", (SensorClient*) this, 5);

	// call timerEvent to fill the status bar with real values
	timerEvent(0);

	/*
	 * Restore size of the dialog box that was used at end of last session.
	 * Due to a bug in Qt we need to set the width to one more than the
	 * defined min width. If this is not done the widget is not drawn
	 * properly the first time. Subsequent redraws after resize are no problem.
	 *
	 * I need to implement a propper session management some day!
	 */
	QString t = Kapp->config()->readEntry(QString("G_Toplevel"));
	if(!t.isNull())
	{
		if (t.length() == 19)
		{ 
			int xpos, ypos, ww, wh;
			sscanf(t.data(), "%04d:%04d:%04d:%04d", &xpos, &ypos, &ww, &wh);
			setGeometry(xpos, ypos,
						ww <= KTOP_MIN_W ? KTOP_MIN_W + 1 : ww,
						wh <= KTOP_MIN_H ? KTOP_MIN_H : wh);
		}
	}

	setMinimumSize(sizeHint());

	readProperties(Kapp->config());

	timerID = startTimer(2000);

	// show the dialog box
	show();

	// switch to the selected startup page
	taskman->raiseStartUpPage();
}

void 
TopLevel::quitSlot()
{
	saveProperties(Kapp->config());

	taskman->saveSettings();

	Kapp->config()->sync();
	qApp->quit();
}

void
TopLevel::timerEvent(QTimerEvent*)
{
	/* Request some info about the memory status. The requested information
	 * will be received by answerReceived(). */
	localhost->sendRequest("pscount", (SensorClient*) this, 0);
	localhost->sendRequest("memfree", (SensorClient*) this, 1);
	localhost->sendRequest("memused", (SensorClient*) this, 2);
	localhost->sendRequest("memswap", (SensorClient*) this, 3);
}

void
TopLevel::answerReceived(int id, const QString& answer)
{
	QString s;
	static QString unit;
	static long mUsed = 0;
	static long mFree = 0;
	static long sTotal = 0;
	static long sFree = 0;

	switch (id)
	{
	case 0:
		s = i18n("%1 Processes").arg(answer);
		statusbar->changeItem(s, 0);
		break;
	case 1:
		mFree = answer.toLong();
		break;
	case 2:
		mUsed = answer.toLong();
		s = i18n("Memory: %1 %2 used, %3 %4 free")
			.arg(mUsed).arg(unit).arg(mFree).arg(unit);
		statusbar->changeItem(s, 1);
		break;
	case 3:
		sFree = answer.toLong();
		s = i18n("Swap: %1 %2 used, %2 %4 free")
			.arg(sTotal - sFree).arg(unit).arg(sFree).arg(unit);
		statusbar->changeItem(s, 2);
		break;
	case 5:
		SensorMonitorInfo info(answer);
		sTotal = info.getMax();
		unit = info.getUnit();
		break;
	}
}

/*
 * Print usage information.
 */
static void 
usage(char *name) 
{
	printf("%s [kdeopts] [--help] [-p (list|perf)]\n", name);
}

/*
 * Where it all begins.
 */
int
main(int argc, char** argv)
{
	// initialize KDE application
	Kapp = new KApplication(argc, argv, "ktop");

	/*
	 * This OSStatus object will be used on platforms that require KTop to
	 * use certain privileges to do it's job.
	 */
	OSStatus priv;

	SensorMgr = new SensorManager();

	int i;
	int sfolder = -1;

	/*
	 * process command line arguments
	 */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i],"--help"))
		{
			// print usage information
			usage(argv[0]);
			return 0;
		}
		if (strstr(argv[i],"-p"))
		{
			// select what page (tab) to show after startup
			i++;
			if (strstr(argv[i], "perf"))
			{
				// performance monitor
				sfolder = TaskMan::PAGE_PERF;
			}
			else if (strstr(argv[i], "list"))
			{
				// process list
				sfolder = TaskMan::PAGE_PLIST;
			}
			else
			{
				// print usage information
				usage(argv[0]);
				return 1;
			}
		}
    }

	if (!priv.ok())
	{
		KMessageBox::error(0, priv.getErrMessage());
		return (-1);
	}

	// create top-level widget
	TopLevel *toplevel = new TopLevel("TaskManager", sfolder);
	Kapp->setMainWidget(toplevel);
	Kapp->setTopWidget(toplevel);
	toplevel->show();

	// run the application
	int result = Kapp->exec();

    delete toplevel;
	delete Kapp;

	return (result);
}
