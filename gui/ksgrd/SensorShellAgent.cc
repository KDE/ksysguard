/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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

#include <kdebug.h>
#include <kprocess.h>
#include <kshell.h>
#include <klocale.h>

//#include "SensorClient.h"
#include "SensorManager.h"

#include "SensorShellAgent.h"

using namespace KSGRD;

SensorShellAgent::SensorShellAgent( SensorManager *sm )
  : SensorAgent( sm ), mDaemon( 0 )
{
}

SensorShellAgent::~SensorShellAgent()
{
  if ( mDaemon ) {
    mDaemon->write( "quit\n", sizeof( "quit\n" )-1 );
    mDaemon->disconnect();
    mDaemon->waitForFinished();
    delete mDaemon;
    mDaemon = 0;
  }
}
	
bool SensorShellAgent::start( const QString &host, const QString &shell,
                              const QString &command, int )
{
  mDaemon = new KProcess();
  mDaemon->setOutputChannelMode( KProcess::SeparateChannels );
  mRetryCount=3;
  setHostName( host );
  mShell = shell;
  mCommand = command;

  connect( mDaemon, SIGNAL(  error ( QProcess::ProcessError  ) ),
           SLOT( daemonError( QProcess::ProcessError ) ) );
  connect( mDaemon, SIGNAL(  finished ( int, QProcess::ExitStatus ) ),
           SLOT( daemonExited( int, QProcess::ExitStatus ) ) );
  connect( mDaemon, SIGNAL( readyReadStandardOutput() ),
           SLOT( msgRcvd() ) );
  connect( mDaemon, SIGNAL( readyReadStandardError() ),
           SLOT( errMsgRcvd() ) );

  if ( !command.isEmpty() ) {
    *mDaemon << KShell::splitArgs(command);
  }
  else
    *mDaemon << mShell << hostName() << "ksysguardd";
  mDaemon->start();

  return true;
}

void SensorShellAgent::hostInfo( QString &shell, QString &command,
                                 int &port) const
{
  shell = mShell;
  command = mCommand;
  port = -1;
}

void SensorShellAgent::msgRcvd( )
{
  QByteArray buffer = mDaemon->readAllStandardOutput();
  mRetryCount = 3; //we received an answer, so reset our retry count back to 3
  processAnswer( buffer.constData(), buffer.size());
}

void SensorShellAgent::errMsgRcvd( )
{
  QByteArray buffer = mDaemon->readAllStandardOutput();

  // Because we read the error buffer in chunks, we may not have a proper utf8 string.
  // We should never get input over stderr anyway, so no need to worry too much about it.
  // But if this is extended, we will need to handle this better
  QString buf = QString::fromUtf8( buffer );

  kDebug(1215) << "SensorShellAgent: Warning, received text over stderr!"
                << endl << buf << endl;
}

void SensorShellAgent::daemonExited(  int exitCode, QProcess::ExitStatus exitStatus )
{
  Q_UNUSED(exitCode);
  kDebug(1215) << "daemon exited, exit status "  << exitStatus;
  if ( mRetryCount--  <= 0 || (mDaemon->start(), !mDaemon->waitForStarted()) )
  {
    setDaemonOnLine( false );
    if(sensorManager()) {
      sensorManager()->disengage( this ); //delete ourselves
    }
  }
}

void SensorShellAgent::daemonError( QProcess::ProcessError errorStatus )
{
  QString error;
  switch(errorStatus) {
    case QProcess::FailedToStart:
      kDebug(1215) << "failed to run" <<  mDaemon->program().join(" ");
      error = i18n("Could not run daemon program '%1'.", mDaemon->program().join(" "));
      break;
    case QProcess::Crashed:
    case QProcess::Timedout:
    case QProcess::WriteError:
    case QProcess::ReadError:
    default:
      error = i18n("The daemon program '%1' failed.", mDaemon->program().join(" "));
  }
  setReasonForOffline(error);
  kDebug(1215) << "Error received " << error << "(" << errorStatus << ")";
  setDaemonOnLine( false );
  if(sensorManager())
    sensorManager()->disengage( this ); //delete ourselves
}
bool SensorShellAgent::writeMsg( const char *msg, int len )
{
  //write returns -1 on error, in which case we should return false.  true otherwise.
  return mDaemon->write( msg, len ) != -1;
}

#include "SensorShellAgent.moc"
