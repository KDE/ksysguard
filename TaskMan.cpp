/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
	Copyright (c) 1999 Chris Schlaeger
	                   cs@axys.de
    
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

#include <config.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/resource.h>       
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <errno.h>
#include <signal.h>

#include <qtabbar.h>
#include <qmessagebox.h>

#include <kapp.h>

#include "ktop.h"
#include "settings.h"
#include "cpu.h"
#include "memory.h"
#include "comm.h"
#include "ReniceDlg.h"
#include "ProcListPage.h"
#include "TaskMan.moc"

#define NONE -1

/*
 * This constructor creates the actual QTabDialog. It is a modeless dialog,
 * using the toplevel widget as its parent, so the dialog won't get its own
 * window.
 */
TaskMan::TaskMan(QWidget* parent, const char* name, int sfolder)
	: QTabDialog(parent, name, FALSE, 0)
{
	pages[0] = pages[1] = pages[2] = NULL;
	settings = NULL;
	restoreStartupPage = FALSE;

	setStyle(WindowsStyle);
    
	connect(tabBar(), SIGNAL(selected(int)), SLOT(tabBarSelected(int)));
     
	/*
	 * set up popup menu pSig
	 */
	pSig = new QPopupMenu(NULL,"_psig");
	CHECK_PTR(pSig);
	pSig->insertItem(i18n("Renice Task..."),MENU_ID_RENICE);
	pSig->insertSeparator();
	pSig->insertItem(i18n("send SIGINT\t(ctrl-c)"), MENU_ID_SIGINT);
	pSig->insertItem(i18n("send SIGQUIT\t(core)"), MENU_ID_SIGQUIT);
	pSig->insertItem(i18n("send SIGTERM\t(term.)"), MENU_ID_SIGTERM);
	pSig->insertItem(i18n("send SIGKILL\t(term.)"), MENU_ID_SIGKILL);
	pSig->insertSeparator();
	pSig->insertItem(i18n("send SIGUSR1\t(user1)"), MENU_ID_SIGUSR1);
	pSig->insertItem(i18n("send SIGUSR2\t(user2)"), MENU_ID_SIGUSR2);
	connect(pSig, SIGNAL(activated(int)), this, SLOT(pSigHandler(int)));
  
    /*
     * set up page 0 (process list viewer)
     */
    pages[0] = procListPage = new ProcListPage(this, "ProcListPage");
    CHECK_PTR(procListPage);
	connect(procListPage, SIGNAL(killProcess(int)),
			this, SLOT(killProcess(int)));
	
	/*
	 * set up page 1 (process tree)
	 */
    pages[1] = procTreePage = new ProcTreePage(this, "ProcTreePage"); 
    CHECK_PTR(procTreePage);
	connect(procTreePage, SIGNAL(killProcess(int)),
			this, SLOT(killProcess(int)));

	/*
	 * set up page 2 (performance monitor)
	 */
    pages[2] = perfMonPage = new PerfMonPage(this, "PerfMonPage");
    CHECK_PTR(perfMonPage);

    // startup_page settings...
	startup_page = PAGE_PLIST;
	QString tmp = Kapp->getConfig()->readEntry("startUpPage");
	if(!tmp.isNull())
		startup_page = tmp.toInt();
	// setting from config file can be overidden by command line option
	if (sfolder >= 0)
	{ 
		restoreStartupPage = TRUE;
		startup_page = sfolder;
	}

    // add pages...
    addTab(procListPage, i18n("Processes &List"));
    addTab(procTreePage, i18n("Processes &Tree"));
    addTab(perfMonPage, i18n("&Performance"));

    move(0,0);
}

void 
TaskMan::raiseStartUpPage()
{
	// tell QTabDialog to raise the specified startup page
	showPage(pages[startup_page]);

	/*
	 * In case the startup_page has been forced on the command line we restore
	 * the startup_page variable form the config file again so we use the
	 * forced value only for this session.
	 */
	if (restoreStartupPage)
	{
		QString tmp = Kapp->getConfig()->readEntry(QString("startUpPage"));
		if(!tmp.isNull())
			startup_page = tmp.toInt();
	}
} 

void 
TaskMan::pSigHandler(int id)
{
	int the_sig = 0;
	bool renice = false;

	// Find out the signal number and the same, or if we need to do a renice.
	/*
	 * I'm not sure whether it makes sense to i18n the signal names. But who
	 * knows. I'll leave that decision to the translators.
	 */
	QString sigName;
	switch (id)
	{
	case MENU_ID_SIGINT:
		the_sig = SIGINT;
		sigName = i18n("SIGINT");
		break;
	case MENU_ID_SIGQUIT:
		the_sig = SIGQUIT;
		sigName = i18n("SIGQUIT");
		break;
	case MENU_ID_SIGTERM:
		the_sig = SIGTERM;
		sigName = i18n("SIGTERM");
		break;
	case MENU_ID_SIGKILL:
		the_sig = SIGKILL;
		sigName = i18n("SIGKILL");
		break;
	case MENU_ID_SIGUSR1:
		the_sig = SIGUSR1;
		sigName = i18n("SIGUSR1");
		break;
	case MENU_ID_SIGUSR2:
		the_sig = SIGUSR2;
		sigName = i18n("SIGUSR2");
		break;
	case MENU_ID_RENICE:
		renice = true;
		break;
	default:
		return;
	}

	// Find out the pid of the currently selected process.
	int pid;
	switch (tabBar()->currentTab())
	{
	case PAGE_PLIST:
		pid = procListPage->selectionPid();
		if (pid == NONE)
			return;
		break;
	case PAGE_PTREE:
		pid = procTreePage->selectionPid();
		if (pid == NONE)
			return;
		break;
	default:
		return;
	}

	// We are about to display a modal dialog so we turn auto update mode off.
	int lastmode = procListPage->setAutoUpdateMode(FALSE);

	if (renice) 
		reniceProcess(pid);
	else
 		killProcess(pid, the_sig, sigName.data());

	// Restore the auto update mode if necessary.
	procListPage->setAutoUpdateMode(lastmode);

	// Update the currently displayed process list/tree.
	switch (tabBar()->currentTab()) 
	{
	case PAGE_PLIST:
		procListPage->update();
		break;
	case PAGE_PTREE:
		procTreePage->update();
		break;
	}
}

void 
TaskMan::tabBarSelected(int tabIndx)
{ 
	switch (tabIndx)
	{
	case PAGE_PLIST:
		procListPage->setAutoUpdateMode(TRUE);
		procListPage->update();
		break;
	case PAGE_PTREE:
		procListPage->setAutoUpdateMode(FALSE);
		procTreePage->update();
		break;
	case PAGE_PERF:
		procListPage->setAutoUpdateMode(FALSE);
		break;
	}
}

void 
TaskMan::killProcess(int pid, int sig, const char* sigName)
{
	// Create a current process list and find process with specified pid.
	OSProcessList pl;
	pl.setSortCriteria(OSProcessList::SORTBY_PID);
	pl.update();
	int idx = pl.find(&OSProcess(pid));
	if (idx < 0)
		return;
	OSProcess* ps = pl.at(idx);

	// Make sure user really want to send the signal to that process.
	QString msg;
	msg.sprintf(i18n("Send signal %s to process %d?\n"
					 "(Process name: %s  Owner: %s)\n"), sigName, ps->getPid(),
				ps->getName(), ps->getUserName().data());
	switch(QMessageBox::warning(this, "ktop", msg,
								i18n("Continue"), i18n("Abort"), 0, 1))
    { 
	case 0: // continue
		if (kill(ps->getPid(), sig))
		{
			QMessageBox::warning(this, "ktop",
								 i18n("Could not send the signal!\n"
								 "The following error occured...\n"),
								 strerror(errno), 0);
		}
		break;

	case 1: // abort
		break;
	}
}

void
TaskMan::reniceProcess(int pid)
{
	/*
	 * -1 is a legitimate priority value as well as the result when an error
	 * has occured. So we have to clear errno to use it as an additional
	 * error indicator.
	 */
	errno = 0;
	// get current priority of the process with pid
	int currentPriority = getpriority(PRIO_PROCESS, pid);
	if (currentPriority == -1 && errno != 0) 
	{
		QMessageBox::warning(this, i18n("ktop"),
							 i18n("Renice error...\n"
								  "Specified process does not exist\n"
								  "or permission denied."),
							 i18n("OK"), 0); 
		return;
	}

	// create a dialog widget
	ReniceDlg dialog(this, "nice", currentPriority);

	// request new priority value with dialog box and set the new priority
	int newPriority;
	if ((newPriority = dialog.exec()) <= 20 && (newPriority >= -20) &&
		(newPriority != currentPriority)) 
	{
		if (setpriority(PRIO_PROCESS, pid, newPriority) == -1)
		{
			QMessageBox::warning(this, i18n("ktop"),
								 i18n("Renice error...\n"
									  "Specified process does not exist\n"
									  "or permission denied."),
								 i18n("OK"), 0);   
		}
	}
}

void 
TaskMan::invokeSettings(void)
{
	// Display and handle preferences dialog.
	if(!settings)
	{
		settings = new AppSettings(0,"proc_options");
		CHECK_PTR(settings);      
	}

	settings->setStartUpPage(startup_page);
	if (settings->exec())
	{
		startup_page = settings->getStartUpPage();
		saveSettings();
	}
}

void 
TaskMan::saveSettings()
{
	QString tmp;

	// save window size
	tmp.sprintf("%04d:%04d:%04d:%04d",
				parentWidget()->x(), parentWidget()->y(),
				parentWidget()->width(), parentWidget()->height());
	Kapp->getConfig()->writeEntry("G_Toplevel", tmp);

	// save startup page (tab)
	Kapp->getConfig()->writeEntry("startUpPage",
					   tmp.setNum(startup_page), TRUE);

	// save process list settings
	procListPage->saveSettings();

	// save process tree settings
	procTreePage->saveSettings();

	Kapp->getConfig()->sync();
}
