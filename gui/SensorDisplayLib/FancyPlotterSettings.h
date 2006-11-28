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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef FANCYPLOTTERSETTINGS_H
#define FANCYPLOTTERSETTINGS_H

#include <QtCore/QList>

#include <kpagedialog.h>

#include "SensorModel.h"

class KColorButton;
class KIntNumInput;
class KLineEdit;

class QCheckBox;
class QPushButton;
class QTreeView;

class FancyPlotterSettings : public KPageDialog
{
  Q_OBJECT

  public:
    FancyPlotterSettings( QWidget* parent, bool locked );
    ~FancyPlotterSettings();

    void setTitle( const QString &title );
    QString title() const;

    void setUseAutoRange( bool value );
    bool useAutoRange() const;

    void setMinValue( double min );
    double minValue() const;

    void setMaxValue( double max );
    double maxValue() const;

    void setHorizontalScale( int scale );
    int horizontalScale() const;

    void setShowVerticalLines( bool value );
    bool showVerticalLines() const;

    void setFontColor( const QColor &color );
    QColor fontColor() const;

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

    void setSensors( const SensorModelEntry::List &list );
    SensorModelEntry::List sensors() const;
    QList<int> order() const;
    QList<int> deleted() const;
    void clearDeleted();
    void resetOrder();


  private Q_SLOTS:
    void editSensor();
    void removeSensor();
    void selectionChanged(const QModelIndex &newCurrent);
    void moveUpSensor();
    void moveDownSensor();

  private:
    KColorButton *mVerticalLinesColor;
    KColorButton *mHorizontalLinesColor;
    KColorButton *mBackgroundColor;
    KColorButton *mFontColor;
    KLineEdit *mMinValue;
    KLineEdit *mMaxValue;
    KLineEdit *mTitle;
    KIntNumInput *mHorizontalScale;
    KIntNumInput *mVerticalLinesDistance;
    KIntNumInput *mHorizontalLinesCount;
    KIntNumInput *mFontSize;

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


    QTreeView *mView;
    SensorModel *mModel;

};

#endif
