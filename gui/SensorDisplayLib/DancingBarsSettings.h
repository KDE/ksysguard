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

#ifndef KSG_DANCINGBARSSETTINGS_H
#define KSG_DANCINGBARSSETTINGS_H

#include <QtCore/QList>

#include <kpagedialog.h>

#include "SensorModel.h"

class KColorButton;
class QDoubleSpinBox;
class KIntNumInput;
class KLineEdit;

class QCheckBox;
class QPushButton;
class QTreeView;

class DancingBarsSettings : public KPageDialog
{
  Q_OBJECT

  public:
    explicit DancingBarsSettings( QWidget* parent = 0, const char* name = 0 );
    ~DancingBarsSettings();

    void setTitle( const QString& title );
    QString title() const;

    void setMinValue( double min );
    double minValue() const;

    void setMaxValue( double max );
    double maxValue() const;

    void setUseLowerLimit( bool value );
    bool useLowerLimit() const;

    void setLowerLimit( double limit );
    double lowerLimit() const;

    void setUseUpperLimit( bool value );
    bool useUpperLimit() const;

    void setUpperLimit( double limit );
    double upperLimit() const;

    void setForegroundColor( const QColor &color );
    QColor foregroundColor() const;

    void setAlarmColor( const QColor &color );
    QColor alarmColor() const;

    void setBackgroundColor( const QColor &color );
    QColor backgroundColor() const;

    void setFontSize( int size );
    int fontSize() const;

    void setSensors( const SensorModelEntry::List &list );
    SensorModelEntry::List sensors() const;

  private Q_SLOTS:
    void editSensor();
    void removeSensor();

  private:
    KColorButton *mForegroundColor;
    KColorButton *mAlarmColor;
    KColorButton *mBackgroundColor;
    QDoubleSpinBox *mMinValue;
    QDoubleSpinBox *mMaxValue;
    QDoubleSpinBox *mLowerLimit;
    QDoubleSpinBox *mUpperLimit;
    KLineEdit *mTitle;
    KIntNumInput *mFontSize;

    QCheckBox *mUseLowerLimit;
    QCheckBox *mUseUpperLimit;
    QPushButton *mEditButton;
    QPushButton *mRemoveButton;

    QTreeView *mView;
    SensorModel *mModel;
};

#endif
