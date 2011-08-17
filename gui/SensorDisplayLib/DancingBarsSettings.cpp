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

#include <kacceleratormanager.h>
#include <kcolorbutton.h>
#include <klineedit.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <knuminput.h>

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTreeView>

#include "DancingBarsSettings.h"

DancingBarsSettings::DancingBarsSettings( QWidget* parent, const char* name )
  : KPageDialog( parent ), mModel( new SensorModel( this ) )
{
  setFaceType( Tabbed );
  setCaption( i18n( "Edit BarGraph Preferences" ) );
  setButtons( Ok | Cancel );
  setObjectName( name );
  setModal( false );

  mModel->setHasLabel( true );

  // Range page
  QFrame *page = new QFrame( this );
  addPage( page, i18n( "Range" ) );
  QGridLayout *pageLayout = new QGridLayout( page );
  pageLayout->setSpacing( spacingHint() );
  pageLayout->setMargin( 0 );

  QGroupBox *groupBox = new QGroupBox( i18n( "Title" ), page );
  QGridLayout *boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );

  mTitle = new KLineEdit( groupBox );
  mTitle->setWhatsThis( i18n( "Enter the title of the display here." ) );
  boxLayout->addWidget( mTitle, 0, 0 );

  pageLayout->addWidget( groupBox, 0, 0 );

  groupBox = new QGroupBox( i18n( "Display Range" ), page );
  boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );
  boxLayout->setColumnStretch( 2, 1 );

  QLabel *label = new QLabel( i18n( "Minimum value:" ), groupBox );
  boxLayout->addWidget( label, 0, 0 );

  mMinValue = new QDoubleSpinBox(groupBox);
  mMinValue->setRange(0, 10000);
  mMinValue->setSingleStep(0.5);
  mMinValue->setValue(0);
  mMinValue->setDecimals(2);
  mMinValue->setWhatsThis( i18n( "Enter the minimum value for the display here. If both values are 0, automatic range detection is enabled." ) );
  boxLayout->addWidget( mMinValue, 0, 1 );
  label->setBuddy( mMinValue );

  label = new QLabel( i18n( "Maximum value:" ), groupBox );
  boxLayout->addWidget( label, 0, 3 );

  mMaxValue = new QDoubleSpinBox( groupBox);
  mMaxValue->setRange(0, 100);
  mMaxValue->setSingleStep(0.5);
  mMaxValue->setValue(100);
  mMaxValue->setDecimals(2);
  mMaxValue->setWhatsThis( i18n( "Enter the maximum value for the display here. If both values are 0, automatic range detection is enabled." ) );
  boxLayout->addWidget( mMaxValue, 0, 4 );
  label->setBuddy( mMaxValue );

  pageLayout->addWidget( groupBox, 1, 0 );

  pageLayout->setRowStretch( 2, 1 );

  // Alarm page
  page = new QFrame( this );
  addPage( page, i18n( "Alarms" ) );
  pageLayout = new QGridLayout( page );
  pageLayout->setSpacing( spacingHint() );
  pageLayout->setMargin( 0 );

  groupBox = new QGroupBox( i18n( "Alarm for Minimum Value" ), page );
  boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );
  boxLayout->setColumnStretch( 1, 1 );

  mUseLowerLimit = new QCheckBox( i18n( "Enable alarm" ), groupBox );
  mUseLowerLimit->setWhatsThis( i18n( "Enable the minimum value alarm." ) );
  boxLayout->addWidget( mUseLowerLimit, 0, 0 );

  label = new QLabel( i18n( "Lower limit:" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mLowerLimit = new QDoubleSpinBox(groupBox);
  mLowerLimit->setRange(0, 100);
  mLowerLimit->setSingleStep(0.5);
  mLowerLimit->setValue(0);
  mLowerLimit->setDecimals(2);
  mLowerLimit->setEnabled( false );
  boxLayout->addWidget( mLowerLimit, 0, 3 );
  label->setBuddy( mLowerLimit );

  pageLayout->addWidget( groupBox, 0, 0 );

  groupBox = new QGroupBox( i18n( "Alarm for Maximum Value" ), page );
  boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );
  boxLayout->setColumnStretch( 1, 1 );

  mUseUpperLimit = new QCheckBox( i18n( "Enable alarm" ), groupBox );
  mUseUpperLimit->setWhatsThis( i18n( "Enable the maximum value alarm." ) );
  boxLayout->addWidget( mUseUpperLimit, 0, 0 );

  label = new QLabel( i18n( "Upper limit:" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mUpperLimit = new QDoubleSpinBox( groupBox);
  mUpperLimit->setRange(0, 1000);
  mUpperLimit->setSingleStep(0.5);
  mUpperLimit->setDecimals(2);
  mUpperLimit->setEnabled( false );
  boxLayout->addWidget( mUpperLimit, 0, 3 );
  label->setBuddy( mUpperLimit );

  pageLayout->addWidget( groupBox, 1, 0 );

  pageLayout->setRowStretch( 2, 1 );

  // Look page
  page = new QFrame( this );
  addPage( page, i18nc( "@title:tab Appearance of the bar graph", "Look" ) );
  pageLayout = new QGridLayout( page );
  pageLayout->setSpacing( spacingHint() );
  pageLayout->setMargin( 0 );

  label = new QLabel( i18n( "Normal bar color:" ), page );
  pageLayout->addWidget( label, 0, 0 );

  mForegroundColor = new KColorButton( page );
  pageLayout->addWidget( mForegroundColor, 0, 1 );
  label->setBuddy( mForegroundColor );

  label = new QLabel( i18n( "Out-of-range color:" ), page );
  pageLayout->addWidget( label, 1, 0 );

  mAlarmColor = new KColorButton( page );
  pageLayout->addWidget( mAlarmColor, 1, 1 );
  label->setBuddy( mAlarmColor );

  label = new QLabel( i18n( "Background color:" ), page );
  pageLayout->addWidget( label, 2, 0 );

  mBackgroundColor = new KColorButton( page );
  pageLayout->addWidget( mBackgroundColor, 2, 1 );
  label->setBuddy( mBackgroundColor );

  label = new QLabel( i18n( "Font size:" ), page );
  pageLayout->addWidget( label, 3, 0 );

  mFontSize = new KIntNumInput( 9, page );
  mFontSize->setWhatsThis( i18n( "This determines the size of the font used to print a label underneath the bars. Bars are automatically suppressed if text becomes too large, so it is advisable to use a small font size here." ) );
  pageLayout->addWidget( mFontSize, 3, 1 );
  label->setBuddy( mFontSize );

  pageLayout->setRowStretch( 4, 1 );

  // Sensor page
  page = new QFrame( this );
  addPage( page, i18n( "Sensors" ) );
  pageLayout = new QGridLayout( page );
  pageLayout->setSpacing( spacingHint() );
  pageLayout->setMargin( 0 );
  pageLayout->setRowStretch( 2, 1 );

  mView = new QTreeView( page );
  mView->header()->setStretchLastSection( true );
  mView->setRootIsDecorated( false );
  mView->setItemsExpandable( false );
  mView->setModel( mModel );
  pageLayout->addWidget( mView, 0, 0, 3, 1);

  mEditButton = new QPushButton( i18n( "Edit..." ), page );
  mEditButton->setWhatsThis( i18n( "Push this button to configure the label." ) );
  pageLayout->addWidget( mEditButton, 0, 1 );

  mRemoveButton = new QPushButton( i18n( "Delete" ), page );
  mRemoveButton->setWhatsThis( i18n( "Push this button to delete the sensor." ) );
  pageLayout->addWidget( mRemoveButton, 1, 1 );

  connect( mUseLowerLimit, SIGNAL(toggled(bool)),
           mLowerLimit, SLOT(setEnabled(bool)) );
  connect( mUseUpperLimit, SIGNAL(toggled(bool)),
           mUpperLimit, SLOT(setEnabled(bool)) );
  connect( mEditButton, SIGNAL(clicked()), SLOT(editSensor()) );
  connect( mRemoveButton, SIGNAL(clicked()), SLOT(removeSensor()) );

  KAcceleratorManager::manage( this );

  mTitle->setFocus();
}

DancingBarsSettings::~DancingBarsSettings()
{
}

void DancingBarsSettings::setTitle( const QString& title )
{
  mTitle->setText( title );
}

QString DancingBarsSettings::title() const
{
  return mTitle->text();
}

void DancingBarsSettings::setMinValue( double min )
{
  mMinValue->setValue( min );
}

double DancingBarsSettings::minValue() const
{
  return mMinValue->value();
}

void DancingBarsSettings::setMaxValue( double max )
{
  mMaxValue->setValue( max );
}

double DancingBarsSettings::maxValue() const
{
  return mMaxValue->value();
}

void DancingBarsSettings::setUseLowerLimit( bool value )
{
  mUseLowerLimit->setChecked( value );
}

bool DancingBarsSettings::useLowerLimit() const
{
  return mUseLowerLimit->isChecked();
}

void DancingBarsSettings::setLowerLimit( double limit )
{
  mLowerLimit->setValue( limit );
}

double DancingBarsSettings::lowerLimit() const
{
  return mLowerLimit->value();
}

void DancingBarsSettings::setUseUpperLimit( bool value )
{
  mUseUpperLimit->setChecked( value );
}

bool DancingBarsSettings::useUpperLimit() const
{
  return mUseUpperLimit->isChecked();
}

void DancingBarsSettings::setUpperLimit( double limit )
{
  mUpperLimit->setValue( limit );
}

double DancingBarsSettings::upperLimit() const
{
  return mUpperLimit->value();
}

void DancingBarsSettings::setForegroundColor( const QColor &color )
{
  mForegroundColor->setColor( color );
}

QColor DancingBarsSettings::foregroundColor() const
{
  return mForegroundColor->color();
}

void DancingBarsSettings::setAlarmColor( const QColor &color )
{
  mAlarmColor->setColor( color );
}

QColor DancingBarsSettings::alarmColor() const
{
  return mAlarmColor->color();
}

void DancingBarsSettings::setBackgroundColor( const QColor &color )
{
  mBackgroundColor->setColor( color );
}

QColor DancingBarsSettings::backgroundColor() const
{
  return mBackgroundColor->color();
}

void DancingBarsSettings::setFontSize( int size )
{
  mFontSize->setValue( size );
}

int DancingBarsSettings::fontSize() const
{
  return mFontSize->value();
}

void DancingBarsSettings::setSensors( const SensorModelEntry::List &list )
{
  mModel->setSensors( list );

  mView->selectionModel()->setCurrentIndex( mModel->index( 0, 0 ), QItemSelectionModel::SelectCurrent |
                                                                   QItemSelectionModel::Rows );
}

SensorModelEntry::List DancingBarsSettings::sensors() const
{
  return mModel->sensors();
}

void DancingBarsSettings::editSensor()
{
  if ( !mView->selectionModel() )
    return;

  const QModelIndex index = mView->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return;

  SensorModelEntry sensor = mModel->sensor( index );

  bool ok;
  const QString name = KInputDialog::getText( i18n( "Label of Bar Graph" ),
                                              i18n( "Enter new label:" ), sensor.label(), &ok, this );
  if ( ok ) {
    sensor.setLabel( name );
    mModel->setSensor( sensor, index );
  }
}

void DancingBarsSettings::removeSensor()
{
  if ( !mView->selectionModel() )
    return;

  const QModelIndex index = mView->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return;

  mModel->removeSensor( index );
}

#include "DancingBarsSettings.moc"
