/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KSysGuard is currently maintained by Chris Schlaeger
	<cs@kde.org>. Please do not commit any changes without consulting
	me first. Thanks!

	KSysGuard has been written with some source code and ideas from
	ktop (<1.0). Early versions of ktop have been written by Bernd
	Johannes Wuebben <wuebben@math.cornell.edu> and Nicolas Leclercq
	<nicknet@planete.net>.  While I tried to preserve their original
	ideas, KSysGuard is a much more powerful tool.

	$Id$
*/

#include <assert.h>
#include <ctype.h>

#include <qstringlist.h>

#include <ktmainwindow.h>
#include <kwinmodule.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kedittoolbar.h>
#include <dcopclient.h>

#include "SensorBrowser.h"
#include "SensorManager.h"
#include "SensorAgent.h"
#include "Workspace.h"
#include "HostConnector.h"
#include "../version.h"

#include "ksysguard.moc"

static const char* Description = I18N_NOOP("KDE System Guard");
TopLevel* Toplevel;

/*
 * This is the constructor for the main widget. It sets up the menu and the
 * TaskMan widget.
 */
TopLevel::TopLevel(const char *name)
	: KTMainWindow(name), DCOPObject("KSysGuardIface")
{
	dontSaveSession = FALSE;

	splitter = new QSplitter(this, "Splitter");
	CHECK_PTR(splitter);
	splitter->setOrientation(Horizontal);
	splitter->setOpaqueResize(TRUE);
	setView(splitter);

	sb = new SensorBrowser(splitter, SensorMgr, "SensorBrowser");
	CHECK_PTR(sb);

	ws = new Workspace(splitter, "Workspace");
	CHECK_PTR(ws);

	/* Create the status bar. It displays some information about the
	 * number of processes and the memory consumption of the local
	 * host. */
	statusbar = new KStatusBar(this, "statusbar");
	CHECK_PTR(statusbar);
	statusbar->insertFixedItem(i18n("88888 Processes"), 0);
	statusbar->insertFixedItem(i18n("Memory: 8888888 kB used, "
							   "8888888 kB free"), 1);
	statusbar->insertFixedItem(i18n("Swap: 8888888 kB used, "
							   "8888888 kB free"), 2);
	setStatusBar(statusbar);
	enableStatusBar(KStatusBar::Hide);

	// call timerEvent to fill the status bar with real values
	timerEvent(0);

	timerID = startTimer(2000);

    // setup File menu
    KStdAction::openNew(ws, SLOT(newWorkSheet()), actionCollection());
    KStdAction::open(ws, SLOT(loadWorkSheet()), actionCollection());
	KStdAction::close(ws, SLOT(deleteWorkSheet()), actionCollection());

	KStdAction::saveAs(ws, SLOT(saveWorkSheetAs()), actionCollection());
	KStdAction::save(ws, SLOT(saveWorkSheet()), actionCollection());
	KStdAction::quit(this, SLOT(quitApp()), actionCollection());

    (void) new KAction(i18n("C&onnect Host"), "connect_established", 0,
					   this, SLOT(connectHost()), actionCollection(),
					   "connect_host");
    (void) new KAction(i18n("D&isconnect Host"), "connect_no", 0, this,
					   SLOT(disconnectHost()), actionCollection(),
					   "disconnect_host");
	toolbarTog = KStdAction::showToolbar(this, SLOT(toggleMainToolBar()),
										 actionCollection(), "showtoolbar");
	toolbarTog->setChecked(FALSE);
	statusBarTog = KStdAction::showStatusbar(this, SLOT(showStatusBar()),
											 actionCollection(),
											 "showstatusbar");
    KStdAction::configureToolbars(this, SLOT(editToolbars()),
								  actionCollection());
	statusBarTog->setChecked(FALSE);
	createGUI("ksysguard.rc");

	show();
}

TopLevel::~TopLevel()
{
	killTimer(timerID);

	delete statusbar;
	delete splitter;
}

/*
 * DCOP Interface functions
 */
void
TopLevel::showProcesses()
{
	ws->showProcesses();
}

void
TopLevel::loadWorkSheet(const QString& fileName)
{
	ws->restoreWorkSheet(fileName);
}

void
TopLevel::removeWorkSheet(const QString& fileName)
{
	ws->deleteWorkSheet(fileName);
}

QStringList
TopLevel::listSensors()
{
	return (sb->listSensors());
}

QString
TopLevel::readSensor(const QString& /*sensorLocator*/)
{
	// TODO: not yet implemented
	return (QString::null);
}

/*
 * End of DCOP Interface section
 */

void
TopLevel::beATaskManager()
{
	ws->showProcesses();

	QValueList<int> sizes;
	sizes.append(0);
	sizes.append(100);
	splitter->setSizes(sizes);

	// Show window centered on the desktop.
	KWinModule kwm;
	QRect workArea = kwm.workArea();
	int w = 600;
	if (workArea.width() < w)
		w = workArea.width();
	int h = 440;
	if (workArea.height() < h)
		h = workArea.height();
	setGeometry((workArea.width() - w) / 2, (workArea.height() - h) / 2,
				w, h);

	// No toolbar and status bar in taskmanager mode.
	enableStatusBar(KStatusBar::Hide);
	statusBarTog->setChecked(FALSE);

	showMainToolBar(FALSE);

	dontSaveSession = TRUE;
}

void
TopLevel::quitApp()
{
	if (!dontSaveSession)
	{
		if (!ws->saveOnQuit())
			return;

		saveProperties(kapp->config());
		kapp->config()->sync();
	}
	qApp->quit();
}

void 
TopLevel::connectHost()
{
	QDialog* d = new HostConnector(0, "HostConnector");
	d->exec();

	delete d;
}

void 
TopLevel::disconnectHost()
{
	sb->disconnect();
}

void
TopLevel::toggleMainToolBar()
{
	KToolBar *toolbar = static_cast<KToolBar *>(
		child("mainToolBar", "KToolBar"));

	if (!toolbar)
		return;
	if (toolbar->isVisible())
		toolbar->hide();
	else
		toolbar->show();
}

void
TopLevel::showStatusBar()
{
	enableStatusBar(KStatusBar::Toggle);
}

void
TopLevel::editToolbars()
{
	KEditToolbar dlg(actionCollection());

	if (dlg.exec())
		createGUI();
}

void
TopLevel::customEvent(QCustomEvent* ev)
{
	if (ev->type() == QEvent::User)
	{
		KMessageBox::error(this, *((QString*) ev->data()));
		delete (QString*) ev->data();
		delete ev;
	}
}

void
TopLevel::timerEvent(QTimerEvent*)
{
	if (statusbar->isVisibleTo(this))
	{
		/* Request some info about the memory status. The requested
		 * information will be received by answerReceived(). */
		SensorMgr->sendRequest("localhost", "pscount", (SensorClient*) this,
							   0);
		SensorMgr->sendRequest("localhost", "mem/physical/free",
							   (SensorClient*) this, 1);
		SensorMgr->sendRequest("localhost", "mem/physical/used",
							   (SensorClient*) this, 2);
		SensorMgr->sendRequest("localhost", "mem/swap/used",
							   (SensorClient*) this, 3);
	}
}

void
TopLevel::readProperties(KConfig* cfg)
{
	cfg->setGroup("KSysGuard Settings");
	
	int ww = cfg->readNumEntry("SizeX", 600);
	int wh = cfg->readNumEntry("SizeY", 375);
	resize(ww, wh);

	QValueList<int> sizes = cfg->readIntListEntry("SplitterSizeList");
	if (sizes.isEmpty())
	{
		// start with a 30/70 ratio
		sizes.append(30);
		sizes.append(70);
	}
	splitter->setSizes(sizes);

	showMainToolBar(!cfg->readNumEntry("ToolBarHidden", 1));

	if (!cfg->readNumEntry("StatusBarHidden", 1))
	{
		enableStatusBar(KStatusBar::Show);
		statusBarTog->setChecked(TRUE);
	}
		
	ws->readProperties(cfg);

	setMinimumSize(sizeHint());

	SensorMgr->engage("localhost", "", "ksysguardd");
	/* Request info about the swapspace size and the units it is measured in.
	 * The requested info will be received by answerReceived(). */
	SensorMgr->sendRequest("localhost", "mem/swap/used?",
						   (SensorClient*) this, 5);
}

void
TopLevel::saveProperties(KConfig* cfg)
{
	cfg->setGroup("KSysGuard Settings");

	// Save window geometry. TODO: x/y is not exaclty correct. Needs fixing.
	cfg->writeEntry("SizeX", width());
	cfg->writeEntry("SizeY", height());
	cfg->writeEntry("SplitterSizeList", splitter->sizes());
	cfg->writeEntry("ToolBarHidden", !toolbarTog->isChecked());
	cfg->writeEntry("StatusBarHidden", !statusBarTog->isChecked());

	ws->saveProperties(cfg);
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
		s = i18n("Swap: %1 %2 used, %3 %4 free")
			.arg(sTotal - sFree).arg(unit).arg(sFree).arg(unit);
		statusbar->changeItem(s, 2);
		break;
	case 5:
		SensorIntegerInfo info(answer);
		sTotal = info.getMax();
		unit = info.getUnit();
		break;
	}
}

void
TopLevel::showMainToolBar(bool show)
{
	KToolBar *toolbar = static_cast<KToolBar *>(
		child("mainToolBar", "KToolBar"));

	if (!toolbar)
		return;
	if (show)
		toolbar->show();
	else
		toolbar->hide();
	toolbarTog->setChecked(show);
}

static const KCmdLineOptions options[] =
{
	{ "showprocesses", I18N_NOOP("show only process list of local host"),
	  0 },
	{ 0, 0, 0}
};


/*
 * Once upon a time...
 */
int
main(int argc, char** argv)
{
	KAboutData aboutData("ksysguard", I18N_NOOP("KDE System Guard"),
						 KSYSGUARD_VERSION, Description,
						 KAboutData::License_GPL,
						 I18N_NOOP("(c) 1996-2000, "
								   "The KSysGuard Developers"));
	aboutData.addAuthor("Chris Schlaeger", "Current Maintainer",
						"cs@kde.org");
	aboutData.addAuthor("Nicolas Leclercq", 0, "nicknet@planete.net");
	aboutData.addAuthor("Alex Sanda", 0, "alex@darkstart.ping.at");
	aboutData.addAuthor("Bernd Johannes Wuebben", 0,
						"wuebben@math.cornell.edu");
	aboutData.addAuthor("Ralf Mueller", 0, "rlaf@bj-ig.de");
	
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);
	
	// initialize KDE application
	KApplication *a = new KApplication;

	SensorMgr = new SensorManager();
	CHECK_PTR(SensorMgr);

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();


	// create top-level widget
	if (a->isRestored())
	{
		RESTORE(TopLevel)
	}
	else
	{
		Toplevel = new TopLevel("KSysGuard");
		if (args->isSet("showprocesses"))
		{
			Toplevel->beATaskManager();
		}
		else
			Toplevel->readProperties(a->config());
		Toplevel->show();
	}
	if (KMainWindow::memberList->first())
	{
		a->dcopClient()->registerAs("ksysguard", FALSE);
		a->dcopClient()->setDefaultObject("KSysGuardIface");
	}

	// run the application
	int result = a->exec();

	return (result);
}
