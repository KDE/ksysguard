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

#include <QtCore/QAbstractTableModel>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QTreeView>
#include <QtGui/QHBoxLayout>
#include <QtXml/QDomNodeList>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knotification.h>
#include <ksgrd/SensorManager.h>
#include <kdebug.h>
#include "StyleEngine.h"

#include "SensorLoggerDlg.h"
#include "SensorLoggerSettings.h"
#include "SensorLogger.h"
#include "LogSensor.h"
#include "SensorDataProvider.h"

#define NONE -1

LogSensorView::LogSensorView( QWidget *parent )
 : QTreeView( parent )
{
}

void LogSensorView::contextMenuEvent( QContextMenuEvent *event )
{
  const QModelIndex index = indexAt( event->pos() );

  emit contextMenuRequest( index, viewport()->mapToGlobal( event->pos() ) );
}

QModelIndexList LogSensorView::selectedIndices()  {
    return QTreeView::selectedIndexes();
}

class LogSensorModel : public QAbstractTableModel
{

  public:
    LogSensorModel( SensorDataProvider* sensorDataProvider, SensorLogger *parent = 0 )
      : QAbstractTableModel( parent )
    {
        mSensorDataProvider = sensorDataProvider;
        connect( parent, SIGNAL( changed() ), this, SIGNAL( layoutChanged() ) );
    }

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const
    {
      Q_UNUSED( parent );

      return 4;
    }


    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const
    {
      Q_UNUSED( parent );

      return mSensorDataProvider->sensorCount();
    }

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
      if ( !index.isValid() )
        return QVariant();

      if ( index.row() >= mSensorDataProvider->sensorCount() || index.row() < 0 )
        return QVariant();

      LogSensor *sensor = static_cast<LogSensor *>(mSensorDataProvider->sensor( index.row() ));

      if ( role == Qt::DisplayRole ) {
        switch (index.column()) {
            case 1:
                return sensor->name();
                break;
            case 2:
                return sensor->hostName();
                break;
            case 3:
                return sensor->fileName();
                break;
            }
      } else if ( role == Qt::DecorationRole ) {
          static QPixmap runningPixmap = KIconLoader::global()->loadIcon( "running", KIconLoader::Small, KIconLoader::SizeSmall );
          static QPixmap waitingPixmap = KIconLoader::global()->loadIcon( "waiting", KIconLoader::Small, KIconLoader::SizeSmall );

          if ( index.column() == 0 ) {
            if ( sensor->isOk() )
              return runningPixmap;
            else
              return waitingPixmap;
          }
      }else if ( role == Qt::ForegroundRole ) {
        if ( sensor->limitReached() )
          return mAlarmColor;
        else
          return mForegroundColor;
      } else if ( role == Qt::BackgroundRole ) {
          return mBackgroundColor;
      }

      return QVariant();
    }

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const
    {
      if ( orientation == Qt::Vertical )
        return QVariant();

      if ( role == Qt::DisplayRole ) {
        switch ( section ) {
          case 0:
            return i18n("Logging");
            break;
          case 1:
            return i18n("Sensor Name");
            break;
          case 2:
            return i18n("Host Name");
            break;
          case 3:
            return i18n("Log File");
            break;
          default:
            return QVariant();
        }
      }

      return QVariant();
    }

    void setForegroundColor( const QColor &color ) { mForegroundColor = color; }
    QColor foregroundColor() const { return mForegroundColor; }

    void setBackgroundColor( const QColor &color ) { mBackgroundColor = color; }
    QColor backgroundColor() const { return mBackgroundColor; }

    void setAlarmColor( const QColor &color ) { mAlarmColor = color; }
    QColor alarmColor() const { return mAlarmColor; }

  private:

    QColor mForegroundColor;
    QColor mBackgroundColor;
    QColor mAlarmColor;

    SensorDataProvider* mSensorDataProvider;
};

SensorLogger::SensorLogger( QWidget *parent, const QString& title, SharedSettings *workSheetSettings )
  : KSGRD::SensorDisplay( parent, title, workSheetSettings )
{
  mModel = new LogSensorModel( sensorDataProvider, this );
  mModel->setForegroundColor( KSGRD::Style->firstForegroundColor() );
  mModel->setBackgroundColor( KSGRD::Style->backgroundColor() );
  mModel->setAlarmColor( KSGRD::Style->alarmColor() );
  
  QLayout *layout = new QHBoxLayout(this);
  mView = new LogSensorView( this );
  layout->addWidget(mView);
  setLayout(layout);


  mView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect(mView, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(showContextMenu(const QPoint &)));

  mView->header()->setStretchLastSection( true );
  mView->setRootIsDecorated( false );
  mView->setItemsExpandable( false );
  mView->setModel( mModel );
  setPlotterWidget( mView );

  QPalette palette = mView->palette();
  palette.setColor( QPalette::Base, KSGRD::Style->backgroundColor() );
  mView->setPalette( palette );

  setTitle( i18n( "Sensor Logger" ) );
  setMinimumSize( 50, 25 );
}

SensorLogger::~SensorLogger(void)
{
}

bool SensorLogger::addSensor( const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& sensorDescr )
{
  Q_UNUSED(sensorType);

  SensorLoggerDlg dlg( this );

  if ( dlg.exec() ) {
    if ( !dlg.fileName().isEmpty() ) {
      LogSensor *sensor = new LogSensor( sensorName,hostName);
      sensor->addTitle(sensorDescr);
      sensor->setFileName( dlg.fileName() );
      sensor->setLowerLimitActive( dlg.lowerLimitActive() );
      sensor->setUpperLimitActive( dlg.upperLimitActive() );
      sensor->setLowerLimit( dlg.lowerLimit() );
      sensor->setUpperLimit( dlg.upperLimit() );

      sensorDataProvider->addSensor(sensor);

      emit changed();
    }
  }

  return true;
}

void SensorLogger::answerReceived(int id, const QList<QByteArray>& answer) {

    LogSensor *currentSensor = static_cast<LogSensor *> (sensor(id));
    QFile mLogFile(currentSensor->fileName());
    if (mLogFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&mLogFile);
        double value = 0;
        if (!answer.isEmpty())
            value = answer[0].toDouble();
        bool isLimitReached = currentSensor->limitReached();
        if (currentSensor->lowerLimitActive() && value < currentSensor->lowerLimit()) {
            currentSensor->setLimitReached(true);
            // send notification
            KNotification::event("sensor_alarm", QString("sensor '%1' at '%2' reached lower limit") .arg(currentSensor->name()).arg(currentSensor->hostName()), QPixmap(), 0);


        } else if (currentSensor->upperLimitActive() && value > currentSensor->upperLimit()) {
            currentSensor->setLimitReached(true);

            // send notification
            KNotification::event("sensor_alarm", QString("sensor '%1' at '%2' reached upper limit") .arg(currentSensor->name()).arg(currentSensor->hostName()), QPixmap(), 0);


        } else
            currentSensor->setLimitReached(false);

        if (isLimitReached != currentSensor->limitReached())
            emit changed();

        const QDate date = QDateTime::currentDateTime().date();
        const QTime time = QDateTime::currentDateTime().time();

        stream << QString("%1 %2 %3 %4 %5: %6\n").arg(date.shortMonthName(date.month())) .arg(date.day()).arg(time.toString()) .arg(currentSensor->hostName()).arg(currentSensor->name()).arg(value);
        mLogFile.close();
        currentSensor->setOk(true);


    } else
        currentSensor->setOk(false);


}

bool SensorLogger::editSensor( LogSensor* sensor )
{
  SensorLoggerDlg dlg( this );

  dlg.setFileName( sensor->fileName() );
  dlg.setLowerLimitActive( sensor->lowerLimitActive() );
  dlg.setLowerLimit( sensor->lowerLimit() );
  dlg.setUpperLimitActive( sensor->upperLimitActive() );
  dlg.setUpperLimit( sensor->upperLimit() );

  if ( dlg.exec() ) {
    if ( !dlg.fileName().isEmpty() ) {
      sensor->setFileName( dlg.fileName() );
      sensor->setLowerLimitActive( dlg.lowerLimitActive() );
      sensor->setUpperLimitActive( dlg.upperLimitActive() );
      sensor->setLowerLimit( dlg.lowerLimit() );
      sensor->setUpperLimit( dlg.upperLimit() );
    }
  }

  return true;
}

void SensorLogger::configureSettings()
{
  SensorLoggerSettings dlg( this );

  dlg.setTitle( title() );
  dlg.setForegroundColor( mModel->foregroundColor() );
  dlg.setBackgroundColor( mModel->backgroundColor() );
  dlg.setAlarmColor( mModel->alarmColor() );

  if ( dlg.exec() ) {
    setTitle( dlg.title() );

    mModel->setForegroundColor( dlg.foregroundColor() );
    mModel->setBackgroundColor( dlg.backgroundColor() );
    mModel->setAlarmColor( dlg.alarmColor() );

    QPalette palette = mView->palette();
    palette.setColor( QPalette::Base, dlg.backgroundColor() );
    mView->setPalette( palette );
  }
}

void SensorLogger::applyStyle()
{
  mModel->setForegroundColor( KSGRD::Style->firstForegroundColor() );
  mModel->setBackgroundColor( KSGRD::Style->backgroundColor() );
  mModel->setAlarmColor( KSGRD::Style->alarmColor() );

  QPalette palette = mView->palette();
  palette.setColor( QPalette::Base, KSGRD::Style->backgroundColor() );
  mView->setPalette( palette );
}

bool SensorLogger::restoreSettings( QDomElement& element )
{
  mModel->setForegroundColor( restoreColor( element, "textColor", Qt::green) );
  mModel->setBackgroundColor( restoreColor( element, "backgroundColor", Qt::black ) );
  mModel->setAlarmColor( restoreColor( element, "alarmColor", Qt::red ) );

  QDomNodeList dnList = element.elementsByTagName( "logsensors" );
  for ( int i = 0; i < dnList.count(); i++ ) {
    QDomElement element = dnList.item( i ).toElement();
    LogSensor* sensor = new LogSensor(element.attribute("sensorName"), element.attribute("hostName")  );

    sensor->setFileName( element.attribute("fileName") );
    sensor->setLowerLimitActive( element.attribute("lowerLimitActive").toInt() );
    sensor->setLowerLimit( element.attribute("lowerLimit").toDouble() );
    sensor->setUpperLimitActive( element.attribute("upperLimitActive").toInt() );
    sensor->setUpperLimit( element.attribute("upperLimit").toDouble() );

    sensorDataProvider->addSensor(sensor);
  }

  SensorDisplay::restoreSettings( element );

  QPalette palette = mView->palette();
  palette.setColor( QPalette::Base, mModel->backgroundColor() );
  mView->setPalette( palette );

  return true;
}

bool SensorLogger::saveSettings( QDomDocument& doc, QDomElement& element )
{
  saveColor( element, "textColor", mModel->foregroundColor() );
  saveColor( element, "backgroundColor", mModel->backgroundColor() );
  saveColor( element, "alarmColor", mModel->alarmColor() );

  int sensorCount = sensorDataProvider->sensorCount();
  for ( int i = 0; i < sensorCount; ++i ) {
    LogSensor *sensor = static_cast<LogSensor *>(sensorDataProvider->sensor(i));
    QDomElement log = doc.createElement( "logsensors" );
    log.setAttribute("sensorName", sensor->name());
    log.setAttribute("hostName", sensor->hostName());
    log.setAttribute("fileName", sensor->fileName());
    log.setAttribute("lowerLimitActive", QString("%1").arg(sensor->lowerLimitActive()));
    log.setAttribute("lowerLimit", QString("%1").arg(sensor->lowerLimit()));
    log.setAttribute("upperLimitActive", QString("%1").arg(sensor->upperLimitActive()));
    log.setAttribute("upperLimit", QString("%1").arg(sensor->upperLimit()));

    element.appendChild( log );
  }

  SensorDisplay::saveSettings( doc, element );

  return true;
}

void SensorLogger::customizeContextMenu(QMenu &pm) {

    QAction *action = 0;

    if (!mSharedSettings->locked && mView->selectedIndices().count() > 0) {
        pm.addSeparator();
        action = pm.addAction(i18n("&Remove Sensor"));
        action->setData(100);

        action = pm.addAction(i18n("&Edit Sensor..."));
        action->setData(101);
    }
}

void SensorLogger::handleCustomizeMenuAction(int id) {
    QModelIndexList selected = mView->selectedIndices();
    switch (id) {
    case 100:
        foreach(QModelIndex mIndex,selected)  {
            removeSensor(mIndex.row());
        }

        break;
    case 101:
        if (selected.count() > 0)
            editSensor(static_cast<LogSensor *>(sensorDataProvider->sensor(selected.at(0).row())));
        break;
    }
}

#include "SensorLogger.moc"
