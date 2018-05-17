/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef KSG_DANCINGBARSSETTINGS_H
#define KSG_DANCINGBARSSETTINGS_H


#include <kpagedialog.h>

#include "SensorModel.h"

class KColorButton;
class QDoubleSpinBox;
class QLineEdit;

class QCheckBox;
class QPushButton;
class QSpinBox;
class QTreeView;

class DancingBarsSettings : public KPageDialog
{
  Q_OBJECT

  public:
    explicit DancingBarsSettings( QWidget* parent = nullptr, const QString &name = QString() );
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

    QList<uint> getDeletedIds();

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
    QLineEdit *mTitle;
    QSpinBox *mFontSize;

    QCheckBox *mUseLowerLimit;
    QCheckBox *mUseUpperLimit;
    QPushButton *mEditButton;
    QPushButton *mRemoveButton;

    QList<uint> deletedIds;

    QTreeView *mView;
    SensorModel *mModel;
};

#endif
