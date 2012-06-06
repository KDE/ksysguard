/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License version 2 or at your option version 3 as published by
    the Free Software Foundation.

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

class QSplitter;
class QAction;
class KAction;
class SensorBrowserWidget;
class Workspace;
class ProcessController;


class TopLevel : public KXmlGuiWindow, public KSGRD::SensorClient
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.SystemMonitor")

  public:
    TopLevel();

    virtual void saveProperties( KConfigGroup& );
    virtual void readProperties( const KConfigGroup& );

    virtual void answerReceived( int id, const QList<QByteArray> & );

    void initStatusBar();
    void setLocalProcessController(ProcessController * localProcessController);
    ProcessController *localProcessController() const { return mLocalProcessController; }

  public Q_SLOTS:
    Q_SCRIPTABLE Q_NOREPLY void showOnCurrentDesktop();
    Q_SCRIPTABLE Q_NOREPLY void importWorkSheet( const QString &fileName );
    Q_SCRIPTABLE Q_NOREPLY void removeWorkSheet( const QString &fileName );
    Q_SCRIPTABLE Q_NOREPLY void getHotNewWorksheet();
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
    void currentTabChanged(int index);
    void updateProcessCount();
    void configureCurrentSheet();

  private:
    void setSwapInfo( qlonglong, qlonglong, const QString& );
    void changeEvent( QEvent * event );
    void retranslateUi();

    QDBusMessage mDBusReply;

    QSplitter* mSplitter;
    void startSensorBrowserWidget();  ///creates an mSensorBrowser if it doesn't exist

    SensorBrowserWidget* mSensorBrowser;
    Workspace* mWorkSpace;

    int mTimerId;
    QAction *mNewWorksheetAction;
    QAction *mInsertWorksheetAction;
    QAction *mTabExportAction;
    QAction *mTabRemoveAction;
    QAction *mMonitorRemoteAction;
    QAction *mHotNewWorksheetAction;
    QAction *mQuitAction;
    QAction *mConfigureSheetAction;
    QAction *mHotNewWorksheetUploadAction;
    KAction *mRefreshTabAction;
    QLabel *sbProcessCount;
    QLabel *sbCpuStat;
    QLabel *sbMemTotal;
    QLabel *sbSwapTotal;
    ProcessController *mLocalProcessController;

    QList<int> mSplitterSize;
};

extern TopLevel* Toplevel;

#endif
