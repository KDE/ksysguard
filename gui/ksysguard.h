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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
    not commit any changes without consulting me first. Thanks!

*/

#ifndef KSG_KSYSGUARD_H
#define KSG_KSYSGUARD_H

#include <qevent.h>

#include <dcopclient.h>
#include <dcopobject.h>
#include <kapplication.h>
#include <kmainwindow.h>

#include <ksgrd/SensorClient.h>

class KRecentFilesAction;
class KToggleAction;

class QSplitter;
class SensorBrowser;
class Workspace;

class TopLevel : public KMainWindow, public KSGRD::SensorClient, public DCOPObject
{
  Q_OBJECT
  K_DCOP

  public:
    TopLevel( const char *name = 0 );

  virtual void saveProperties( KConfig* );
  virtual void readProperties( KConfig* );

  virtual void answerReceived( int id, const QString& );

  void beATaskManager();
  void showRequestedSheets();
  void initStatusBar();

  k_dcop:
    // calling ksysguard with kwin/kicker hot-key
    ASYNC showProcesses();
    ASYNC showOnCurrentDesktop();
    ASYNC loadWorkSheet( const QString &fileName );
    ASYNC removeWorkSheet( const QString &fileName );
    QStringList listHosts();
    QStringList listSensors( const QString &hostName );
    QString readIntegerSensor( const QString &sensorLocator );
    QStringList readListSensor( const QString &sensorLocator );

  public slots:
    void registerRecentURL( const KURL &url );
    void resetWorkSheets();

  protected:
    virtual void customEvent( QCustomEvent* );
    virtual void timerEvent( QTimerEvent* );
    virtual bool queryClose();

  protected slots:
    void connectHost();
    void disconnectHost();
    void showStatusBar();
    void editToolbars();
    void editStyle();
    void slotNewToolbarConfig();

  private:
    QPtrList<DCOPClientTransaction> mDCopFIFO;

    QSplitter* mSplitter;
    KRecentFilesAction* mActionOpenRecent;
    KToggleAction* mActionStatusBar;

    SensorBrowser* mSensorBrowser;
    Workspace* mWorkSpace;

    bool mDontSaveSession;
    int mTimerId;
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
