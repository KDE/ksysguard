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

class KColorButton;
class KIntNumInput;
class KLineEdit;

class QCheckBox;
class QPushButton;
class QTreeView;

class SensorModel;

class SensorEntry
{
  public:
    typedef QList<SensorEntry> List;

    void setId( int id ) { mId = id; }
    int id() const { return mId; }

    void setHostName( const QString &hostName ) { mHostName = hostName; }
    QString hostName() const { return mHostName; }

    void setSensorName( const QString &sensorName ) { mSensorName = sensorName; }
    QString sensorName() const { return mSensorName; }

    void setUnit( const QString &unit ) { mUnit = unit; }
    QString unit() const { return mUnit; }

    void setStatus( const QString &status ) { mStatus = status; }
    QString status() const { return mStatus; }

    void setColor( const QColor &color ) { mColor = color; }
    QColor color() const { return mColor; }

  private:
    int mId;
    QString mHostName;
    QString mSensorName;
    QString mUnit;
    QString mStatus;
    QColor mColor;
};

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

    void setSensors( const SensorEntry::List &list );
    SensorEntry::List sensors() const;

  private Q_SLOTS:
    void editSensor();
    void removeSensor();

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

    QCheckBox *mShowVerticalLines;
    QCheckBox *mShowHorizontalLines;
    QCheckBox *mVerticalLinesScroll;
    QCheckBox *mUseAutoRange;
    QCheckBox *mShowLabels;
    QCheckBox *mShowTopBar;
    QPushButton *mEditButton;
    QPushButton *mRemoveButton;

    QTreeView *mView;
    SensorModel *mModel;
};

#endif
