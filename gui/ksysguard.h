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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _ktop_h_
#define _ktop_h_

#include <qpopupmenu.h>
#include <qsplitter.h>
#include <qevent.h>

#include <kapp.h>
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <dcopobject.h>
#include <dcopclient.h>

#include "SensorClient.h"

class KRecentFilesAction;
class KToggleAction;
class SensorAgent;
class SensorBrowser;
class Workspace;

class TopLevel : public KMainWindow, public SensorClient, public DCOPObject
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
	ASYNC loadWorkSheet(const QString& fileName);
	ASYNC removeWorkSheet(const QString& fileName);
	QStringList listHosts();
	QStringList listSensors(const QString& hostName);
	QString readIntegerSensor(const QString& sensorLocator);
	QStringList readListSensor(const QString& sensorLocator);

public slots:
	void registerRecentURL(const KURL& url);

protected:
	virtual void customEvent(QCustomEvent* e);
	virtual void timerEvent(QTimerEvent*);
	virtual bool queryClose();

protected slots:
	void connectHost();
	void disconnectHost();
	void toggleMainToolBar();
	void showStatusBar();
	void editToolbars();

private:
	void showMainToolBar(bool show);

	KStatusBar* statusbar;

	QList<DCOPClientTransaction> dcopFIFO;

	QSplitter* splitter;
	KRecentFilesAction* openRecent;
	KToggleAction* toolbarTog;
	KToggleAction* statusBarTog;
	SensorBrowser* sb;
	Workspace* ws;
	bool dontSaveSession;
	int timerId;
};

extern TopLevel* Toplevel;

/*
   since there is only a forward declaration of DCOPClientTransaction
   in dcopclient.h we have to redefine it here, otherwise QList
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
