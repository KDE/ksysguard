/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>
    
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

#ifndef KSG_WORKSHEET_H
#define KSG_WORKSHEET_H

#include <QWidget>
#include <QTimer>

#include <SensorDisplay.h>
#include "SharedSettings.h"

class QDomElement;
class QDragEnterEvent;
class QDropEvent;
class QGridLayout;
class QString;
class QStringList;

/**
  A WorkSheet contains the displays to visualize the sensor results. When
  creating the WorkSheet you must specify the number of columns. Displays
  can be added and removed on the fly. The grid layout will handle the
  layout. The number of columns can not be changed. Displays are added by
  dragging a sensor from the sensor browser over the WorkSheet.
 */
class WorkSheet : public QWidget
{
  Q_OBJECT

  public:
    explicit WorkSheet( QWidget* parent);
    WorkSheet( uint rows, uint columns, float interval, QWidget* parent);
    ~WorkSheet();

    bool load( const QString &fileName );
    bool save( const QString &fileName );
    bool exportWorkSheet( const QString &fileName );

    void cut();
    void copy();
    void paste();

    void setFileName( const QString &fileName );
    QString fileName() const;

    QString fullFileName() const;

    bool isLocked() const {return mSharedSettings.locked;}

    QString title() const;
    QString translatedTitle() const;

    KSGRD::SensorDisplay* addDisplay( const QString &hostname,
                                      const QString &monitor,
                                      const QString &sensorType,
                                      const QString &sensorDescr,
                                      uint rows, uint columns );

    void settings();

  public Q_SLOTS:
    void showPopupMenu( KSGRD::SensorDisplay *display );
    void setTitle( const QString &title );
    void applyStyle();

  Q_SIGNALS:
    void titleChanged( QWidget *sheet );

  protected:

    virtual void changeEvent( QEvent * event );
    virtual QSize sizeHint() const;
    virtual void dragMoveEvent( QDragMoveEvent* );
    virtual void dragEnterEvent( QDragEnterEvent* );
    void dropEvent( QDropEvent* );
    bool event( QEvent* );
    void setUpdateInterval( float interval);
    float updateInterval() const;

  private:
    void removeDisplay( KSGRD::SensorDisplay *display );

    bool replaceDisplay( uint row, uint column, QDomElement& element );

    void replaceDisplay( uint row, uint column,
                         KSGRD::SensorDisplay* display = 0 );

    void collectHosts( QStringList &list );

    void createGrid( uint rows, uint columns );

    void resizeGrid( uint rows, uint columns );

    KSGRD::SensorDisplay* currentDisplay( uint* row = 0, uint* column = 0 );

    void fixTabOrder();

    QString currentDisplayAsXML();

    uint mRows;
    uint mColumns;

    QGridLayout* mGridLayout;
    QString mFileName;
    QString mFullFileName;
    QString mTitle;
    QString mTranslatedTitle;

    SharedSettings mSharedSettings;

    QTimer mTimer;

    enum DisplayType { DisplayDummy, DisplayFancyPlotter, DisplayMultiMeter, DisplayDancingBars, DisplaySensorLogger, DisplayListView, DisplayLogFile, DisplayProcessControllerRemote, DisplayProcessControllerLocal };

    KSGRD::SensorDisplay* insertDisplay( DisplayType displayType, uint row, uint column);
    /**
      This two dimensional array stores the pointers to the sensor displays
	    or if no sensor is present at a position a pointer to a dummy widget.
  	  The size of the array corresponds to the size of the grid layout.
     */
    KSGRD::SensorDisplay*** mDisplayList;
};

#endif
