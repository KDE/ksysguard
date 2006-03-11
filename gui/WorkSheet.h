/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>
    
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

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#ifndef KSG_WORKSHEET_H
#define KSG_WORKSHEET_H

#include <qwidget.h>

#include <SensorDisplay.h>

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
class WorkSheet : public QWidget, public KSGRD::SensorBoard
{
  Q_OBJECT

  public:
    WorkSheet( QWidget* parent);
    WorkSheet( uint rows, uint columns, uint interval, QWidget* parent);
    ~WorkSheet();

    bool load( const QString &fileName );
    bool save( const QString &fileName );

    void cut();
    void copy();
    void paste();

    void setFileName( const QString &fileName );
    const QString& fileName() const;

    bool modified() const;

    void setTitle( const QString &title );
    const QString &title();

    KSGRD::SensorDisplay* addDisplay( const QString &hostname,
                                      const QString &monitor,
                                      const QString &sensorType,
                                      const QString &sensorDescr,
                                      uint rows, uint columns );

    void settings();

    void setIsOnTop( bool onTop );

  public Q_SLOTS:
    void showPopupMenu( KSGRD::SensorDisplay *display );
    void setModified( bool mfd );
    void applyStyle();

  Q_SIGNALS:
    void sheetModified( QWidget *sheet );
    void titleChanged( QWidget *sheet );

  protected:
    virtual QSize sizeHint() const;
    void dragEnterEvent( QDragEnterEvent* );
    void dropEvent( QDropEvent* );
    void customEvent( QCustomEvent* );

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

    bool mModified;

    uint mRows;
    uint mColumns;

    QGridLayout* mGridLayout;
    QString mFileName;
    QString mTitle;

    /**
      This two dimensional array stores the pointers to the sensor displays
	    or if no sensor is present at a position a pointer to a dummy widget.
  	  The size of the array corresponds to the size of the grid layout.
     */
    KSGRD::SensorDisplay*** mDisplayList;
};

#endif
