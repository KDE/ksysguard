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
*/

// $Id$

#include <assert.h>

#include <qmessagebox.h>

#include <ktmainwindow.h>
#include <kconfig.h>
#include <klocale.h>

#include "OSStatus.h"
#include "ktop.moc"

//#define KTOP_MIN_W	549
//#define KTOP_MIN_H	446
#define KTOP_MIN_W	40
#define KTOP_MIN_H	40

/*
 * Global variables
 */
KApplication* Kapp;

/*
 * This is the constructor for the main widget. It sets up the menu and the
 * TaskMan widget.
 */
TopLevel::TopLevel(QWidget *parent, const char *name, int sfolder)
	: KTMainWindow(name)
{
	/*
	 * create main menu
	 */

	menubar = new MainMenu(this, "MainMenu");
	connect(menubar, SIGNAL(quit()), this, SLOT(quitSlot()));
	// register the menu bar with KTMainWindow
	setMenu(menubar);

	statusbar = new KStatusBar(this, "statusbar");
	statusbar->insertItem(i18n("88888 Processes"), 0);
	statusbar->insertItem(i18n("Memory: 888888 kB used, "
							   "888888 kB free"), 1);
	statusbar->insertItem(i18n("Swap: 888888 kB used, "
							   "888888 kB free"), 2);
	setStatusBar(statusbar);
	// call timerEvent to fill the status bar with real values
	timerEvent(0);

	assert(Kapp);
	setCaption(i18n("KDE Task Manager"));
	setMinimumSize(KTOP_MIN_W, KTOP_MIN_H);

	// create the tab dialog
	taskman = new TaskMan(this, "", sfolder);

	// register the tab dialog with KTMainWindow as client widget
	setView(taskman);

	connect(taskman, SIGNAL(enableRefreshMenu(bool)),
			menubar, SLOT(enableRefreshMenu(bool)));

//	setMinimumSize(sizeHint());

	/*
	 * Restore size of the dialog box that was used at end of last session.
	 * Due to a bug in Qt we need to set the width to one more than the
	 * defined min width. If this is not done the widget is not drawn
	 * properly the first time. Subsequent redraws after resize are no problem.
	 */
	QString t = Kapp->getConfig()->readEntry(QString("G_Toplevel"));
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
	else 
		setGeometry(0,0, KTOP_MIN_W + 1, KTOP_MIN_H);

	timerID = startTimer(2000);

	// show the dialog box
    show();

	// switch to the selected startup page
    taskman->raiseStartUpPage();
}

void 
TopLevel::quitSlot()
{
	taskman->saveSettings();
	Kapp->getConfig()->sync();
	qApp->quit();
}

void
TopLevel::timerEvent(QTimerEvent*)
{
	QString s;

	s.sprintf(i18n("%d Processes"), osStatus.getNoProcesses());
	statusbar->changeItem(s, 0);

	int dum, mUsed, mFree;
	osStatus.getMemoryInfo(dum, mFree, mUsed, dum, dum);
	s.sprintf(i18n("Memory: %d kB used, %d kB free"), mUsed, mFree);
	statusbar->changeItem(s, 1);

	int sTotal, sFree;
	osStatus.getSwapInfo(sTotal, sFree);
	s.sprintf(i18n("Swap: %d kB used, %d kB free"), sTotal - sFree, sFree);
	statusbar->changeItem(s, 2);
}

/*
 * Print usage information.
 */
static void 
usage(char *name) 
{
	printf("%s [kdeopts] [--help] [-p (list|tree|perf)]\n", name);
}

/*
 * Where it all begins.
 */
int
main(int argc, char** argv)
{
	// initialize KDE application
	Kapp = new KApplication(argc, argv, "ktop");
	Kapp->enableSessionManagement(true);

	/*
	 * This OSStatus object will be used on platforms that require KTop to
	 * use certain privileges to do it's job.
	 */
	OSStatus priv;

	int i;
	int sfolder = -1;

	/*
	 * process command line arguments
	 */
	for (i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i],"--help"))
		{
			// print usage information
			usage(argv[0]);
			return 0;
		}
		if(strstr(argv[i],"-p"))
		{
			// select what page (tab) to show after startup
			i++;
			if ( strstr(argv[i], "perf"))
			{
				// performance monitor
				sfolder = TaskMan::PAGE_PERF;
			}
			else if (strstr(argv[i], "list"))
			{
				// process list
				sfolder = TaskMan::PAGE_PLIST;
			}
			else if (strstr(argv[i], "tree"))
			{
				// process tree
				sfolder=TaskMan::PAGE_PTREE;
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
		QMessageBox::critical(NULL, "KDE Task Manager", priv.getErrMessage(),
							  0, 0);
		return (-1);
	}

	// create top-level widget
	QWidget *toplevel = new TopLevel(0, "TaskManager", sfolder);
	Kapp->setTopWidget(toplevel);

	// run the application
	int result = Kapp->exec();

    delete toplevel;
	delete Kapp;

	return (result);
}
