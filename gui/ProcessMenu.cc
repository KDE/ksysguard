/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
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

	$Id$
*/


#include <signal.h>
#include <assert.h>

#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "ProcessMenu.moc"
#include "ReniceDlg.h"

ProcessMenu::ProcessMenu(QWidget* parent, const char* name)
	: QPopupMenu(parent, name)
{
	selectedProcess = -1;

#if 0
	OSProcessList pl;

	if (pl.hasNiceLevel())
	{
		insertItem(i18n("Renice Task..."), MENU_ID_RENICE);
		insertSeparator();
	}
#endif
	insertItem(i18n("send SIGINT\t(ctrl-c)"), MENU_ID_SIGINT);
	insertItem(i18n("send SIGQUIT\t(core)"), MENU_ID_SIGQUIT);
	insertItem(i18n("send SIGTERM\t(term.)"), MENU_ID_SIGTERM);
	insertItem(i18n("send SIGKILL\t(term.)"), MENU_ID_SIGKILL);
	insertSeparator();
	insertItem(i18n("send SIGUSR1\t(user1)"), MENU_ID_SIGUSR1);
	insertItem(i18n("send SIGUSR2\t(user2)"), MENU_ID_SIGUSR2);

	connect(this, SIGNAL(activated(int)), SLOT(handler(int)));
}

void
ProcessMenu::handler(int id)
{
	if (id == MENU_ID_RENICE) 
		reniceProcess(selectedProcess);
	else
		killProcess(selectedProcess, id);

	emit(requestUpdate());
}

void 
ProcessMenu::killProcess(int /* pid*/, int /*sig*/)
{
#if 0
	if (pid < 0)
	{
		KMessageBox::sorry(this, i18n("You need to select a process first!"));
		return;
	}

	OSProcess ps(pid);

	if (!ps.ok())
	{
		QString msg;
		msg = i18n("The process %1 is no longer persistent.").arg(pid);
		KMessageBox::sorry(this, msg);
		return;
	}

	/*
	 * I'm not sure whether it makes sense to i18n the signal names. But who
	 * knows. I'll leave that decision to the translators.
	 */
	QString sigName;
	switch (sig)
	{
	case SIGINT:
		sigName = i18n("SIGINT");
		break;
	case SIGQUIT:
		sigName = i18n("SIGQUIT");
		break;
	case SIGTERM:
		sigName = i18n("SIGTERM");
		break;
	case SIGKILL:
		sigName = i18n("SIGKILL");
		break;
	case SIGUSR1:
		sigName = i18n("SIGUSR1");
		break;
	case SIGUSR2:
		sigName = i18n("SIGUSR2");
		break;
	default:
		sigName = i18n("Unkown Signal");
		break;
	}

	// Make sure user really want to send the signal to that process.
	QString msg = i18n("Send signal %1 to process %2?\n"
			   "(Process name: %3  Owner: %4)\n")
		.arg(sigName)
		.arg(ps.getPid())
		.arg(ps.getName())
		.arg(ps.getUserName());
	switch(KMessageBox::questionYesNo(this, msg))
    { 
	case 0: // yes
		if (!ps.sendSignal(sig))
			KMessageBox::error(this, ps.getErrMessage());
		break;

	case 1: // no
		break;
	}
#endif
}

void
ProcessMenu::reniceProcess(int /*pid*/)
{
#if 0
	assert(pid >= 0);

	OSProcess ps(pid);

	if (!ps.ok())
	{
		QString msg = i18n("The process %1 is no longer persistent.").arg(pid);
		KMessageBox::sorry(this, msg);
		return;
	}

	// get current priority of the process with pid
	int currentNiceLevel = ps.getNiceLevel();
	if (!ps.ok()) 
	{
		KMessageBox::sorry(this, i18n("Renice error...\n"
					      "Specified process does not exist\n"
					      "or permission denied."));
		return;
	}

	// create a dialog widget
	ReniceDlg dialog(this, "nice", currentNiceLevel, pid);

	// request new nice value with dialog box and set the new nice level
	int newNiceLevel;
	if ((newNiceLevel = dialog.exec()) <= 20 && (newNiceLevel >= -20) &&
		(newNiceLevel != currentNiceLevel)) 
	{
		if (!ps.setNiceLevel(newNiceLevel))
		{
			KMessageBox::sorry(this, i18n("Renice error...\n"
						"Specified process does not exist\n"
						"or permission denied."));
		}
	}
#endif
}
