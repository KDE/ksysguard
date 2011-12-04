/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef SENSORLOGGER_H
#define SENSORLOGGER_H

#include <QtGui/QTreeView>

#include <SensorDisplay.h>

class LogSensorModel;
class QDomElement;

class LogSensor : public QObject, public KSGRD::SensorClient
{
  Q_OBJECT

  public:
    explicit LogSensor( QObject *parent );
    ~LogSensor();

    virtual void answerReceived( int id, const QList<QByteArray>&answer );

    void setHostName( const QString& name );
    QString hostName() const;

    void setSensorName( const QString& name );
    QString sensorName() const;

    void setFileName( const QString& name );
    QString fileName() const;

    void setUpperLimitActive( bool value );
    bool upperLimitActive() const;

    void setLowerLimitActive( bool value );
    bool lowerLimitActive() const;

    void setUpperLimit( double value );
    double upperLimit() const;

    void setLowerLimit( double value );
    double lowerLimit() const;

    void setTimerInterval( int interval );
    int timerInterval() const;

    bool isLogging() const;

    bool limitReached() const;

  public Q_SLOTS:
    void timerOff();
    void timerOn();

    void startLogging();
    void stopLogging();

  Q_SIGNALS:
    void changed();

  protected:
    virtual void timerTick();
    virtual void timerEvent ( QTimerEvent * event );

  private:
    QString mSensorName;
    QString mHostName;
    QString mFileName;

    int mTimerInterval;
    int mTimerID;

    bool mLowerLimitActive;
    bool mUpperLimitActive;

    double mLowerLimit;
    double mUpperLimit;

    bool mLimitReached;
};

class LogSensorView : public QTreeView
{
  Q_OBJECT

  public:
    LogSensorView( QWidget *parent = 0 );

  Q_SIGNALS:
    void contextMenuRequest( const QModelIndex &index, const QPoint &pos );

  protected:
    virtual void contextMenuEvent( QContextMenuEvent *event );
};

class SensorLogger : public KSGRD::SensorDisplay
{
  Q_OBJECT

  public:
    SensorLogger( QWidget *parent, const QString& title, SharedSettings *workSheetSettings );
    ~SensorLogger();

    bool addSensor( const QString& hostName, const QString& sensorName,
                    const QString& sensorType, const QString& sensorDescr);

    bool editSensor( LogSensor* );

    virtual void answerReceived( int, const QList<QByteArray>& );

    bool restoreSettings( QDomElement& );
    bool saveSettings( QDomDocument&, QDomElement& );

    void configureSettings();

    virtual bool hasSettingsDialog() const
    {
      return true;
    }

  public Q_SLOTS:
    void applyStyle();
    void contextMenuRequest( const QModelIndex &index, const QPoint &pos );

  private:
    LogSensorModel *mModel;
    LogSensorView *mView;
};

#endif
