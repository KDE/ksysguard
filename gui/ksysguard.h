/*
    KSysGuard, the KDE System Guard

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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _ktop_h_
#define _ktop_h_

#include <qevent.h>
#include <qpopupmenu.h>
#include <qsplitter.h>

#include <dcopclient.h>
#include <dcopobject.h>
#include <kapplication.h>
#include <kmainwindow.h>
#include <kstatusbar.h>

#include <ksgrd/SensorClient.h>

class KRecentFilesAction;
class KToggleAction;
class SensorBrowser;
class Workspace;

class TopLevel : public KMainWindow, public KSGRD::SensorClient, public DCOPObject
{
	Q_OBJECT
	K_DCOP

public:
	TopLevel(const char *name = 0);
	~TopLevel();

	virtual void saveProperties(KConfig*);
	virtual void readProperties(KConfig*);

	virtual void answerReceived(int id, const QString& s);

	void beATaskManager();
	void showRequestedSheets();
	void initStatusBar();

k_dcop:
	// calling ksysguard with kwin/kicker hot-key
	ASYNC showProcesses();
	ASYNC showOnCurrentDesktop();
	ASYNC loadWorkSheet(const QString& fileName);
	ASYNC removeWorkSheet(const QString& fileName);
	QStringList listHosts();
	QStringList listSensors(const QString& hostName);
	QString readIntegerSensor(const QString& sensorLocator);
	QStringList readListSensor(const QString& sensorLocator);

public slots:
	void registerRecentURL(const KURL& url);
	void resetWorkSheets();

protected:
	virtual void customEvent(QCustomEvent* e);
	virtual void timerEvent(QTimerEvent*);
	virtual bool queryClose();

protected slots:
	void connectHost();
	void disconnectHost();
	void showStatusBar();
	void editToolbars();
	void editStyle();
	void slotNewToolbarConfig();
private:
	KStatusBar* statusbar;

	QPtrList<DCOPClientTransaction> dcopFIFO;

	QSplitter* splitter;
	KRecentFilesAction* openRecent;
	KToggleAction* statusBarTog;
	SensorBrowser* sb;
	Workspace* ws;
	bool dontSaveSession;
	int timerId;
};

extern TopLevel* Toplevel;

/*
   since there is only a forward declaration of DCOPClientTransaction
   in dcopclient.h we have to redefine it here, otherwise QPtrList
   causes errors
*/
typedef unsigned long CARD32;

class DCOPClientTransaction
{
public:
	Q_INT32 id;
	CARD32 key;
	QCString senderId;
};

#endif
