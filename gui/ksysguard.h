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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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
    void readIntegerSensor( const QString &sensorLocator );
    void readListSensor( const QString &sensorLocator );

  public Q_SLOTS:
    void registerRecentURL( const KUrl &url );
    void resetWorkSheets();

  protected:
    virtual void customEvent( QCustomEvent* );
    virtual void timerEvent( QTimerEvent* );
    virtual bool queryClose();

  protected Q_SLOTS:
    void connectHost();
    void disconnectHost();
    void updateStatusBar();
    void editToolbars();
    void editStyle();
    void slotNewToolbarConfig();

  private:
    void setSwapInfo( long, long, const QString& );

    QList<DCOPClientTransaction *> mDCopFIFO;

    QSplitter* mSplitter;
    KRecentFilesAction* mActionOpenRecent;

    SensorBrowser* mSensorBrowser;
    Workspace* mWorkSpace;

    bool mDontSaveSession;
    int mTimerId;
};

extern TopLevel* Toplevel;

/*
   since there is only a forward declaration of DCOPClientTransaction
   in dcopclient.h we have to redefine it here, otherwise Q3PtrList
   causes errors
*/
typedef unsigned long CARD32;

class DCOPClientTransaction
{
  public:
    qint32 id;
    CARD32 key;
    QByteArray senderId;
};

#endif
