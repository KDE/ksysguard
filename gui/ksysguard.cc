/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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
	<nicknet@planete.net>.

	$Id$
*/

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <kaction.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <kstandarddirs.h>

#include <ksgrd/SensorAgent.h>
#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include "../version.h"
#include "ksysguard.moc"
#include "SensorBrowser.h"
#include "Workspace.h"

static const char* Description = I18N_NOOP("KDE System Guard");
TopLevel* Toplevel;

/*
 * This is the constructor for the main widget. It sets up the menu and the
 * TaskMan widget.
 */
TopLevel::TopLevel(const char *name)
	: KMainWindow(0, name), DCOPObject("KSysGuardIface")
{
	setPlainCaption(i18n("KDE System Guard"));
	dontSaveSession = false;
	timerId = -1;

	splitter = new QSplitter(this, "Splitter");
	Q_CHECK_PTR(splitter);
	splitter->setOrientation(Horizontal);
	splitter->setOpaqueResize(true);
	setCentralWidget(splitter);

	sb = new SensorBrowser(splitter, KSGRD::SensorMgr, "SensorBrowser");
	Q_CHECK_PTR(sb);

	ws = new Workspace(splitter, "Workspace");
	Q_CHECK_PTR(ws);
	connect(ws, SIGNAL(announceRecentURL(const KURL&)),
			this, SLOT(registerRecentURL(const KURL&)));
	connect(ws, SIGNAL(setCaption(const QString&, bool)),
			this, SLOT(setCaption(const QString&, bool)));
	connect(KSGRD::Style, SIGNAL(applyStyleToWorksheet()), ws, SLOT(applyStyle()));

	/* Create the status bar. It displays some information about the
	 * number of processes and the memory consumption of the local
	 * host. */
	statusbar = statusBar();

	statusbar->insertFixedItem(i18n("88888 Processes"), 0);
	statusbar->insertFixedItem(i18n("Memory: 8888888 kB used, "
							   "8888888 kB free"), 1);
	statusbar->insertFixedItem(i18n("Swap: 8888888 kB used, "
							   "8888888 kB free"), 2);
	statusbar->hide();

	// create actions for menue entries
	KStdAction::openNew(ws, SLOT(newWorkSheet()), actionCollection());
	KStdAction::open(ws, SLOT(loadWorkSheet()), actionCollection());
	openRecent = KStdAction::openRecent(ws, SLOT(loadWorkSheet(const KURL&)),
										actionCollection());
	KStdAction::close(ws, SLOT(deleteWorkSheet()), actionCollection());

	KStdAction::saveAs(ws, SLOT(saveWorkSheetAs()), actionCollection());
	KStdAction::save(ws, SLOT(saveWorkSheet()), actionCollection());
	KStdAction::quit(this, SLOT(close()), actionCollection());

	(void) new KAction(i18n("C&onnect Host..."), "connect_established", 0,
					   this, SLOT(connectHost()), actionCollection(),
					   "connect_host");
	(void) new KAction(i18n("D&isconnect Host"), "connect_no", 0, this,
					   SLOT(disconnectHost()), actionCollection(),
					   "disconnect_host");
	KStdAction::cut(ws, SLOT(cut()), actionCollection());
	KStdAction::copy(ws, SLOT(copy()), actionCollection());
	KStdAction::paste(ws, SLOT(paste()), actionCollection());
	(void) new KAction(i18n("&Work Sheet Properties..."), "configure", 0, ws,
					   SLOT(configure()), actionCollection(),
					   "configure_sheet");
	KStdAction::revert(this, SLOT( resetWorkSheets() ), actionCollection() );
	KStdAction::showToolbar("mainToolBar", actionCollection());
	statusBarTog = KStdAction::showStatusbar(this, SLOT(showStatusBar()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(editToolbars()), actionCollection());
	statusBarTog->setChecked(false);
	(void) new KAction(i18n("Configure &Style..."), "colorize", 0, this,
					   SLOT(editStyle()), actionCollection(),
					   "configure_style");
	resize(600, 440);

	createGUI();
}

TopLevel::~TopLevel()
{
}

/*
 * DCOP Interface functions
 */


void TopLevel::resetWorkSheets()
{
	if ( KMessageBox::questionYesNo( this,
			i18n( "Do you really want restore the default work sheets?" ),
			i18n( "Reset all work sheets" ),
			KStdGuiItem::yes(), KStdGuiItem::no(),
			"AskResetWorkSheets") == KMessageBox::No )
		return;

	ws->removeAllWorkSheets();

	KStandardDirs* kstd = KGlobal::dirs();
	kstd->addResourceType("data", "share/apps/ksysguard");

	QString workDir = kstd->saveLocation("data", "ksysguard");

	QString f = kstd->findResource("data", "SystemLoad.sgrd");
	QString fNew = workDir + "/" + i18n("System Load") + ".sgrd";
	if (!f.isEmpty())
		ws->restoreWorkSheet(f, fNew);

	f = kstd->findResource("data", "ProcessTable.sgrd");
	fNew = workDir + "/" + i18n("Process Table") + ".sgrd";
	if (!f.isEmpty())
		ws->restoreWorkSheet(f, fNew);
}


void
TopLevel::showProcesses()
{
	ws->showProcesses();
}

void
TopLevel::showOnCurrentDesktop()
{
	KWin::setOnDesktop( winId(), KWin::currentDesktop() );
}

void
TopLevel::loadWorkSheet(const QString& fileName)
{
	ws->loadWorkSheet(KURL(fileName));
}

void
TopLevel::removeWorkSheet(const QString& fileName)
{
	ws->deleteWorkSheet(fileName);
}

QStringList
TopLevel::listSensors(const QString& hostName)
{
	return (sb->listSensors(hostName));
}

QStringList
TopLevel::listHosts()
{
	return (sb->listHosts());
}

QString
TopLevel::readIntegerSensor(const QString& sensorLocator)
{
	QString host = sensorLocator.left(sensorLocator.find(':'));
	QString sensor = sensorLocator.right(sensorLocator.length() -
										 sensorLocator.find(':') - 1);

	DCOPClientTransaction *dcopTransaction =
		kapp->dcopClient()->beginTransaction();
	dcopFIFO.prepend(dcopTransaction);

	KSGRD::SensorMgr->engage(host, "", "ksysguardd");
	KSGRD::SensorMgr->sendRequest(host, sensor, (KSGRD::SensorClient*) this, 133);

	return (QString::null);
}

QStringList
TopLevel::readListSensor(const QString& sensorLocator)
{
	QStringList retval;
	
	QString host = sensorLocator.left(sensorLocator.find(':'));
	QString sensor = sensorLocator.right(sensorLocator.length() -
										 sensorLocator.find(':') - 1);

	DCOPClientTransaction *dcopTransaction =
		kapp->dcopClient()->beginTransaction();
	dcopFIFO.prepend(dcopTransaction);

	KSGRD::SensorMgr->engage(host, "", "ksysguardd");
	KSGRD::SensorMgr->sendRequest(host, sensor, (KSGRD::SensorClient*) this, 134);

	return retval;
}

/*
 * End of DCOP Interface section
 */

void
TopLevel::registerRecentURL(const KURL& url)
{
	openRecent->addURL(url);
}

void
TopLevel::beATaskManager()
{
	ws->showProcesses();

	// Avoid displaying splitter widget
	sb->hide();

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
	toolBar("mainToolBar")->hide();
	statusBarTog->setChecked(false);
	showStatusBar();

	dontSaveSession = true;

	stateChanged("showProcessState");
}

void
TopLevel::showRequestedSheets()
{
	toolBar("mainToolBar")->hide();

	QValueList<int> sizes;
	sizes.append(0);
	sizes.append(100);
	splitter->setSizes(sizes);

}

void
TopLevel::initStatusBar()
{
	KSGRD::SensorMgr->engage("localhost", "", "ksysguardd");
	/* Request info about the swap space size and the units it is
	 * measured in.  The requested info will be received by
	 * answerReceived(). */
	KSGRD::SensorMgr->sendRequest("localhost", "mem/swap/used?",
						   (KSGRD::SensorClient*) this, 5);
}

void 
TopLevel::connectHost()
{
	KSGRD::SensorMgr->engageHost("");
}

void 
TopLevel::disconnectHost()
{
	sb->disconnect();
}

void
TopLevel::showStatusBar()
{
	if (statusBarTog->isChecked())
	{
		statusBar()->show();
		if (timerId == -1)
		{
			timerId = startTimer(2000);
		}
		// call timerEvent to fill the status bar with real values
		timerEvent(0);
	}
	else
	{
		statusBar()->hide();
		if (timerId != -1)
		{
			killTimer(timerId);
			timerId = -1;
		} 
	}
}

void
TopLevel::editToolbars()
{
	KEditToolbar dlg(actionCollection());

	bool isHidden = toolBar("mainToolBar")->isHidden();

	// Is it a bug, that createGUI() show the toolbar even when it's hidden?
	if (dlg.exec())
	{
		createGUI();
		if (isHidden)
			toolBar("mainToolBar")->hide();
	}
}

void
TopLevel::editStyle()
{
	KSGRD::Style->configure();
}

void
TopLevel::customEvent(QCustomEvent* ev)
{
	if (ev->type() == QEvent::User)
	{
		/* Due to the asynchronous communication between ksysguard and its
		 * back-ends, we sometimes need to show message boxes that were
		 * triggered by objects that have died already. */
		KMessageBox::error(this, *((QString*) ev->data()));
		delete (QString*) ev->data();
	}
}

void
TopLevel::timerEvent(QTimerEvent*)
{
	if (statusbar->isVisibleTo(this))
	{
		/* Request some info about the memory status. The requested
		 * information will be received by answerReceived(). */
		KSGRD::SensorMgr->sendRequest("localhost", "pscount", (KSGRD::SensorClient*) this,
							   0);
		KSGRD::SensorMgr->sendRequest("localhost", "mem/physical/free",
							   (KSGRD::SensorClient*) this, 1);
		KSGRD::SensorMgr->sendRequest("localhost", "mem/physical/used",
							   (KSGRD::SensorClient*) this, 2);
		KSGRD::SensorMgr->sendRequest("localhost", "mem/swap/free",
							   (KSGRD::SensorClient*) this, 3);
	}
}

bool
TopLevel::queryClose()
{
	if (!dontSaveSession)
	{
		if (!ws->saveOnQuit())
			return (false);

		saveProperties(kapp->config());
		kapp->config()->sync();
	}

	return (true);
}

void
TopLevel::readProperties(KConfig* cfg)
{
	/* we can ignore 'isMaximized' because we can't set the window
	   maximized, so we save the coordinates instead */
	if (cfg->readBoolEntry("isMinimized") == true)
		showMinimized();
	else {
		int wx = cfg->readNumEntry("PosX", 100);
		int wy = cfg->readNumEntry("PosY", 100);
		int ww = cfg->readNumEntry("SizeX", 600);
		int wh = cfg->readNumEntry("SizeY", 375);
		setGeometry(wx, wy, ww, wh);
	}

	QValueList<int> sizes = cfg->readIntListEntry("SplitterSizeList");
	if (sizes.isEmpty())
	{
		// start with a 30/70 ratio
		sizes.append(30);
		sizes.append(70);
	}
	splitter->setSizes(sizes);

	applyMainWindowSettings(cfg);

	if (!statusBar()->isHidden())
	{
		statusBarTog->setChecked(true);
		showStatusBar();
	}

	KSGRD::SensorMgr->readProperties(cfg);
	KSGRD::Style->readProperties(cfg);

	ws->readProperties(cfg);

	setMinimumSize(sizeHint());

	openRecent->loadEntries(cfg);
}

void
TopLevel::saveProperties(KConfig* cfg)
{
	openRecent->saveEntries(cfg);

	cfg->writeEntry("PosX", x());
	cfg->writeEntry("PosY", y());
	cfg->writeEntry("SizeX", width());
	cfg->writeEntry("SizeY", height());
	cfg->writeEntry("isMinimized", isMinimized());
	cfg->writeEntry("SplitterSizeList", splitter->sizes());

	saveMainWindowSettings(cfg);

	KSGRD::Style->saveProperties(cfg);
	KSGRD::SensorMgr->saveProperties(cfg);

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
		// yes, I know there is never 1 process, but that's the way
		// singular vs. plural works :/
		//
		// To use pluralForms, though, you need to convert to
		// an integer, not use the QString straight.
		s = i18n("1 Process", "%n Processes", answer.toInt());
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
	{
		KSGRD::SensorIntegerInfo info(answer);
		sTotal = info.getMax();
		unit = KSGRD::SensorMgr->translateUnit(info.getUnit());
		break;
	}
	case 133:
	{
		QCString replyType = "QString";
		QByteArray replyData;
		QDataStream reply(replyData, IO_WriteOnly);
		reply << answer;

		DCOPClientTransaction *dcopTransaction = dcopFIFO.last();
		kapp->dcopClient()->endTransaction(dcopTransaction, replyType,
										   replyData);
		dcopFIFO.removeLast();
		break;
	}

	case 134:
	{
		QStringList resultList;
		QCString replyType = "QStringList";
		QByteArray replyData;
		QDataStream reply(replyData, IO_WriteOnly);

		KSGRD::SensorTokenizer lines(answer, '\n');

		for (unsigned int i = 0; i < lines.numberOfTokens(); i++)
			resultList.append(lines[i]);

		reply << resultList;

		DCOPClientTransaction *dcopTransaction = dcopFIFO.last();
		kapp->dcopClient()->endTransaction(dcopTransaction, replyType,
										   replyData);
		dcopFIFO.removeLast();
		break;
	}
	}
}

static const KCmdLineOptions options[] =
{
	{ "showprocesses", I18N_NOOP("Show only process list of local host"), 0 },
	{ "+[worksheet]", I18N_NOOP("Optional worksheet files to load"), 0 },
	{ 0, 0, 0}
};

/*
 * Once upon a time...
 */
int
main(int argc, char** argv)
{
	// initpipe is used to keep the parent process around till the child
	// has registered with dcop. 
	int initpipe[2];
	pipe(initpipe);

	/* This forking will put ksysguard in it's on session not having a
	 * controlling terminal attached to it. This prevents ssh from
	 * using this terminal for password requests. Unfortunately you
	 * now need a ssh with ssh-askpass support to popup an X dialog to
	 * enter the password. Currently only the original ssh provides this
	 * but not open-ssh. */
	 
	pid_t pid;
	if ((pid = fork()) < 0)
		return (-1);
	else
		if (pid != 0)
		{
			close(initpipe[1]);

			// wait till init is complete
			char c;
			while( read(initpipe[0], &c, 1) < 0);

			// then exit
			close(initpipe[0]);
			exit(0);
		}
	close(initpipe[0]);
	setsid();

	KAboutData aboutData("ksysguard", I18N_NOOP("KDE System Guard"),
						 KSYSGUARD_VERSION, Description,
						 KAboutData::License_GPL,
						 I18N_NOOP("(c) 1996-2002 The KSysGuard Developers"));
	aboutData.addAuthor("Chris Schlaeger", "Current Maintainer",
						"cs@kde.org");
	aboutData.addAuthor("Tobias Koenig", 0, "tokoe82@yahoo.de");
	aboutData.addAuthor("Nicolas Leclercq", 0, "nicknet@planete.net");
	aboutData.addAuthor("Alex Sanda", 0, "alex@darkstart.ping.at");
	aboutData.addAuthor("Bernd Johannes Wuebben", 0,
						"wuebben@math.cornell.edu");
	aboutData.addAuthor("Ralf Mueller", 0, "rlaf@bj-ig.de");
	aboutData.addAuthor("Hamish Rodda", 0, "meddie@yoyo.cc.monash.edu.au");
	aboutData.addAuthor("Torsten Kasch", 
		"Solaris Support\n"
		"Parts derived (by permission) from the sunos5 module of William LeFebvre's \"top\" utility.",
		"tk@Genetik.Uni-Bielefeld.DE");
	
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);
	
	KApplication::disableAutoDcopRegistration();
	// initialize KDE application
	KApplication *a = new KApplication;

	KSGRD::SensorMgr = new KSGRD::SensorManager();
	Q_CHECK_PTR(KSGRD::SensorMgr);
	KSGRD::Style = new KSGRD::StyleEngine();
	Q_CHECK_PTR(KSGRD::Style);

	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

	int result = 0;

	if (args->isSet("showprocesses"))
	{
		/* To avoid having multiple instances of ksysguard in
		 * taskmanager mode we check if another taskmanager is running
		 * already. If so, we terminate this one immediately. */
		if (a->dcopClient()->registerAs("ksysguard_taskmanager", false) ==
			"ksysguard_taskmanager")
		{
			// We have registered with DCOP, our parent can exit now.
			char c = 0;
			write(initpipe[1], &c, 1);
			close(initpipe[1]);

			Toplevel = new TopLevel("KSysGuard");
			Toplevel->beATaskManager();
			Toplevel->show();
			KSGRD::SensorMgr->setBroadcaster(Toplevel);

			// run the application
			result = a->exec();
		}
		else
		{
			QByteArray data;
			a->dcopClient()->send( "ksysguard_taskmanager", "KSysGuardIface", "showOnCurrentDesktop()", data );
		}
	}
	else
	{
		a->dcopClient()->registerAs("ksysguard");
		a->dcopClient()->setDefaultObject("KSysGuardIface");

		// We have registered with DCOP, our parent can exit now.
		char c = 0;
		write(initpipe[1], &c, 1);
		close(initpipe[1]);

		Toplevel = new TopLevel("KSysGuard");
		Q_CHECK_PTR(Toplevel);

		// create top-level widget
		if (args->count() > 0)
		{
			/* The user has specified a list of worksheets to load. In this
			 * case we do not restore any previous settings but load all the
			 * requested worksheets. */
			Toplevel->showRequestedSheets();
			for (int i = 0; i < args->count(); ++i)
				Toplevel->loadWorkSheet(args->arg(i));
		}
		else
		{
			if (a->isRestored())
				Toplevel->restore(1);
			else
				Toplevel->readProperties(a->config());
		}

		Toplevel->initStatusBar();
		Toplevel->show();
		KSGRD::SensorMgr->setBroadcaster(Toplevel);

		// run the application
		result = a->exec();
	}

	delete KSGRD::Style;
	delete KSGRD::SensorMgr;
	delete a;

	return (result);
}
