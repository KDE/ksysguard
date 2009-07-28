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
#include <LogSensor.h>

class LogSensorModel;
class QDomElement;

class LogSensorView : public QTreeView
{
  Q_OBJECT

  public:
    LogSensorView( QWidget *parent = 0 );

    virtual QModelIndexList selectedIndices();

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
  Q_SIGNALS:
    void changed();

  protected:
    virtual void customizeContextMenu(QMenu &);
    virtual void handleCustomizeMenuAction(int id);

  private:
    LogSensorModel *mModel;
    LogSensorView *mView;
};

#endif
