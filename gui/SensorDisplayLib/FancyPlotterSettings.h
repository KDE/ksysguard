/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#ifndef FANCYPLOTTERSETTINGS_H
#define FANCYPLOTTERSETTINGS_H

#include <kdialogbase.h>

class KColorButton;
class KIntNumInput;
class KLineEdit;
class KListView;

class QCheckBox;
class QListViewItem;
class QPushButton;
class QRadioButton;

class FancyPlotterSettings : public KDialogBase
{
  Q_OBJECT

  public:
    FancyPlotterSettings( QWidget* parent = 0, const char* name = 0 );
    ~FancyPlotterSettings();

    void setTitle( const QString &title );
    QString title() const;

    void setUseAutoRange( bool value );
    bool useAutoRange() const;

    void setMinValue( double min );
    double minValue() const;

    void setMaxValue( double max );
    double maxValue() const;

    void setUsePolygonStyle( bool value );
    bool usePolygonStyle() const;

    void setHorizontalScale( int scale );
    int horizontalScale() const;

    void setShowVerticalLines( bool value );
    bool showVerticalLines() const;

    void setVerticalLinesColor( const QColor &color );
    QColor verticalLinesColor() const;

    void setVerticalLinesDistance( int distance );
    int verticalLinesDistance() const;

    void setVerticalLinesScroll( bool value );
    bool verticalLinesScroll() const;

    void setShowHorizontalLines( bool value );
    bool showHorizontalLines() const;

    void setHorizontalLinesColor( const QColor &color );
    QColor horizontalLinesColor() const;

    void setHorizontalLinesCount( int count );
    int horizontalLinesCount() const;

    void setShowLabels( bool value );
    bool showLabels() const;

    void setShowTopBar( bool value );
    bool showTopBar() const;

    void setFontSize( int size );
    int fontSize() const;

    void setBackgroundColor( const QColor &color );
    QColor backgroundColor() const;

    void setSensors( const QValueList< QStringList > &list );
    QValueList< QStringList > sensors() const;

  private slots:
    void editSensor();
    void removeSensor();
    void moveUpSensor();
    void moveDownSensor();
    void selectionChanged( QListViewItem* );

  private:

    KColorButton *mVerticalLinesColor;
    KColorButton *mHorizontalLinesColor;
    KColorButton *mBackgroundColor;
    KLineEdit *mMinValue;
    KLineEdit *mMaxValue;
    KLineEdit *mTitle;
    KIntNumInput *mHorizontalScale;
    KIntNumInput *mVerticalLinesDistance;
    KIntNumInput *mHorizontalLinesCount;
    KIntNumInput *mFontSize;
    KListView *mSensorView;

    QCheckBox *mShowVerticalLines;
    QCheckBox *mShowHorizontalLines;
    QCheckBox *mVerticalLinesScroll;
    QCheckBox *mUseAutoRange;
    QCheckBox *mShowLabels;
    QCheckBox *mShowTopBar;
    QPushButton *mEditButton;
    QPushButton *mRemoveButton;
    QPushButton *mMoveUpButton;
    QPushButton *mMoveDownButton;
    QRadioButton *mUsePolygonStyle;
    QRadioButton *mUseOriginalStyle;
};

#endif
