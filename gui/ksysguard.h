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
*/

#ifndef KSG_KSYSGUARD_H
#define KSG_KSYSGUARD_H

#include <QEvent>
#include <QtDBus/QtDBus>

#include <kapplication.h>
#include <kxmlguiwindow.h>

#include <ksgrd/SensorClient.h>

class KToggleAction;

class QSplitter;
class SensorBrowserWidget;
class Workspace;

class TopLevel : public KXmlGuiWindow, public KSGRD::SensorClient
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.SystemMonitor")

  public:
    TopLevel();

    virtual void saveProperties( KConfigGroup& );
    virtual void readProperties( const KConfigGroup& );

    virtual void answerReceived( int id, const QList<QByteArray> & );

    void beATaskManager();
    void initStatusBar();

  public Q_SLOTS:
     // calling ksysguard with kwin/kicker hot-key
    Q_SCRIPTABLE Q_NOREPLY void showOnCurrentDesktop();
    Q_SCRIPTABLE Q_NOREPLY void importWorkSheet( const QString &fileName );
    Q_SCRIPTABLE Q_NOREPLY void removeWorkSheet( const QString &fileName );
    Q_SCRIPTABLE QStringList listHosts();
    Q_SCRIPTABLE QStringList listSensors( const QString &hostName );

  protected:
    virtual bool event( QEvent* );
    virtual void timerEvent( QTimerEvent* );
    virtual bool queryClose();

  protected Q_SLOTS:
    void connectHost();
    void disconnectHost();
    void updateStatusBar();
    void editToolbars();
    void slotNewToolbarConfig();
    void currentTabChanged(int index);

  private:
    void setSwapInfo( long, long, const QString& );

    QDBusMessage mDBusReply;

    QSplitter* mSplitter;
    void startSensorBrowserWidget();  ///creates an mSensorBrowser if it doesn't exist

    SensorBrowserWidget* mSensorBrowser;
    Workspace* mWorkSpace;

    int mTimerId;
    QAction *mTabRemoveAction;
    QAction *mTabExportAction;
    QAction *mMonitorRemoteAction;
    QList<int> mSplitterSize;
};

extern TopLevel* Toplevel;

#endif
