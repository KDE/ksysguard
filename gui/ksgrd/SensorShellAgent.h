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

#ifndef KSG_SENSORSHELLAGENT_H
#define KSG_SENSORSHELLAGENT_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QProcess>

#include "SensorAgent.h"

class QString;

class KProcess;

namespace KSGRD {

class SensorManager;

/**
  The SensorShellAgent starts a ksysguardd process and handles the
  asynchronous communication. It keeps a list of pending requests
  that have not been answered yet by ksysguard. The current
  implementation only allowes one pending requests. Incoming requests
  are queued in an input FIFO.
 */
class SensorShellAgent : public SensorAgent
{
  Q_OBJECT

  public:
    explicit SensorShellAgent( SensorManager *sm );
    ~SensorShellAgent();

    bool start( const QString &host, const QString &shell,
                const QString &command = "", int port = -1 );

    void hostInfo( QString &shell, QString &command, int &port) const;

  private Q_SLOTS:
    void msgRcvd( );
    void errMsgRcvd( );
    void daemonExited(  int exitCode, QProcess::ExitStatus exitStatus );
    void daemonError( QProcess::ProcessError errorStatus );

  private:
    bool writeMsg( const char *msg, int len );
    int mRetryCount;
    QPointer<KProcess> mDaemon;
    QString mShell;
    QString mCommand;
};

}
	
#endif
