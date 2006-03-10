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

#include <kacceleratormanager.h>
#include <kcolorbutton.h>
#include <klineedit.h>
#include <kinputdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <knuminput.h>

#include <qcheckbox.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

//Added by qt3to4:
#include <QList>
#include <QGridLayout>

#include "DancingBarsSettings.h"

DancingBarsSettings::DancingBarsSettings( QWidget* parent, const char* name )
  : KDialogBase( Tabbed, i18n( "Edit BarGraph Preferences" ), 
    Ok | Apply | Cancel, Ok, parent, name, true, true )
{
  // Range page
  QFrame *page = addPage( i18n( "Range" ) );
  QGridLayout *pageLayout = new QGridLayout( page, 3, 1, 0, spacingHint() );

  Q3GroupBox *groupBox = new Q3GroupBox( 0, Qt::Vertical, i18n( "Title" ), page );
  QGridLayout *boxLayout = new QGridLayout( groupBox->layout(), 1, 1 );

  mTitle = new KLineEdit( groupBox );
  mTitle->setWhatsThis( i18n( "Enter the title of the display here." ) );
  boxLayout->addWidget( mTitle, 0, 0 );

  pageLayout->addWidget( groupBox, 0, 0 );

  groupBox = new Q3GroupBox( 0, Qt::Vertical, i18n( "Display Range" ), page );
  boxLayout = new QGridLayout( groupBox->layout(), 1, 5 );
  boxLayout->setColStretch( 2, 1 );

  QLabel *label = new QLabel( i18n( "Minimum value:" ), groupBox );
  boxLayout->addWidget( label, 0, 0 );

  mMinValue = new KDoubleSpinBox( 0, 10000, 0.5, 0, groupBox, 2 );
  mMinValue->setWhatsThis( i18n( "Enter the minimum value for the display here. If both values are 0 automatic range detection is enabled." ) );
  boxLayout->addWidget( mMinValue, 0, 1 );
  label->setBuddy( mMinValue );

  label = new QLabel( i18n( "Maximum value:" ), groupBox );
  boxLayout->addWidget( label, 0, 3 );

  mMaxValue = new KDoubleSpinBox( 0, 100, 0.5, 100, groupBox, 2 );
  mMaxValue->setWhatsThis( i18n( "Enter the maximum value for the display here. If both values are 0 automatic range detection is enabled." ) );
  boxLayout->addWidget( mMaxValue, 0, 4 );
  label->setBuddy( mMaxValue );

  pageLayout->addWidget( groupBox, 1, 0 );

  pageLayout->setRowStretch( 2, 1 );

  // Alarm page
  page = addPage( i18n( "Alarms" ) );
  pageLayout = new QGridLayout( page, 3, 1, 0, spacingHint() );

  groupBox = new Q3GroupBox( 0, Qt::Vertical, i18n( "Alarm for Minimum Value" ), page );
  boxLayout = new QGridLayout( groupBox->layout(), 1, 4 );
  boxLayout->setColStretch( 1, 1 );

  mUseLowerLimit = new QCheckBox( i18n( "Enable alarm" ), groupBox );
  mUseLowerLimit->setWhatsThis( i18n( "Enable the minimum value alarm." ) );
  boxLayout->addWidget( mUseLowerLimit, 0, 0 );

  label = new QLabel( i18n( "Lower limit:" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mLowerLimit = new KDoubleSpinBox( 0, 100, 0.5, 0, groupBox, 2 );
  mLowerLimit->setEnabled( false );
  boxLayout->addWidget( mLowerLimit, 0, 3 );
  label->setBuddy( mLowerLimit );

  pageLayout->addWidget( groupBox, 0, 0 );

  groupBox = new Q3GroupBox( 0, Qt::Vertical, i18n( "Alarm for Maximum Value" ), page );
  boxLayout = new QGridLayout( groupBox->layout(), 1, 4 );
  boxLayout->setColStretch( 1, 1 );

  mUseUpperLimit = new QCheckBox( i18n( "Enable alarm" ), groupBox );
  mUseUpperLimit->setWhatsThis( i18n( "Enable the maximum value alarm." ) );
  boxLayout->addWidget( mUseUpperLimit, 0, 0 );

  label = new QLabel( i18n( "Upper limit:" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mUpperLimit = new KDoubleSpinBox( 0, 100, 0.5, 0, groupBox, 2 );
  mUpperLimit->setEnabled( false );
  boxLayout->addWidget( mUpperLimit, 0, 3 );
  label->setBuddy( mUpperLimit );

  pageLayout->addWidget( groupBox, 1, 0 );

  pageLayout->setRowStretch( 2, 1 );

  // Look page
  page = addPage( i18n( "Look" ) );
  pageLayout = new QGridLayout( page, 5, 2, 0, spacingHint() );

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
  page = addPage( i18n( "Sensors" ) );
  pageLayout = new QGridLayout( page, 3, 2, 0, spacingHint() );
  pageLayout->setRowStretch( 2, 1 );

  mSensorView = new KListView( page );
  mSensorView->addColumn( i18n( "Host" ) );
  mSensorView->addColumn( i18n( "Sensor" ) );
  mSensorView->addColumn( i18n( "Label" ) );
  mSensorView->addColumn( i18n( "Unit" ) );
  mSensorView->addColumn( i18n( "Status" ) );
  mSensorView->setAllColumnsShowFocus( true );
  pageLayout->addMultiCellWidget( mSensorView, 0, 2, 0, 0 );

  mEditButton = new QPushButton( i18n( "Edit..." ), page );
  mEditButton->setEnabled( false );
  mEditButton->setWhatsThis( i18n( "Push this button to configure the label." ) );
  pageLayout->addWidget( mEditButton, 0, 1 );

  mRemoveButton = new QPushButton( i18n( "Delete" ), page );
  mRemoveButton->setEnabled( false );
  mRemoveButton->setWhatsThis( i18n( "Push this button to delete the sensor." ) );
  pageLayout->addWidget( mRemoveButton, 1, 1 );

  connect( mUseLowerLimit, SIGNAL( toggled( bool ) ),
           mLowerLimit, SLOT( setEnabled( bool ) ) );
  connect( mUseUpperLimit, SIGNAL( toggled( bool ) ),
           mUpperLimit, SLOT( setEnabled( bool ) ) );

  connect( mSensorView, SIGNAL( selectionChanged( Q3ListViewItem* ) ),
           SLOT( selectionChanged( Q3ListViewItem* ) ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( editSensor() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeSensor() ) );

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

void DancingBarsSettings::setSensors( const QList< QStringList > &list )
{
  mSensorView->clear();

  QList< QStringList >::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    new Q3ListViewItem( mSensorView,
                       (*it)[ 0 ],   // host name
                       (*it)[ 1 ],   // sensor name
                       (*it)[ 2 ],   // footer title
                       (*it)[ 3 ],   // unit
                       (*it)[ 4 ] ); // status
  }
}

QList< QStringList > DancingBarsSettings::sensors() const
{
  QList< QStringList > list;

  Q3ListViewItemIterator it( mSensorView );
  while ( it.current() && !it.current()->text( 0 ).isEmpty() ) {
    QStringList entry;
    entry << it.current()->text( 0 );
    entry << it.current()->text( 1 );
    entry << it.current()->text( 2 );
    entry << it.current()->text( 3 );
    entry << it.current()->text( 4 );

    list.append( entry );
    ++it;
  }

  return list;
}

void DancingBarsSettings::editSensor()
{
  Q3ListViewItem *lvi = mSensorView->currentItem();

  if ( !lvi )
    return;

  bool ok;
  QString str = KInputDialog::getText( i18n( "Label of Bar Graph" ),
    i18n( "Enter new label:" ), lvi->text( 2 ), &ok, this );
  if ( ok )
    lvi->setText( 2, str );
}

void DancingBarsSettings::removeSensor()
{
  Q3ListViewItem *lvi = mSensorView->currentItem();

  if ( lvi ) {
    /* Before we delete the currently selected item, we determine a
     * new item to be selected. That way we can ensure that multiple
     * items can be deleted without forcing the user to select a new
     * item between the deletes. If all items are deleted, the buttons
     * are disabled again. */
    Q3ListViewItem* newSelected = 0;
    if ( lvi->itemBelow() ) {
      lvi->itemBelow()->setSelected( true );
      newSelected = lvi->itemBelow();
    } else if ( lvi->itemAbove() ) {
      lvi->itemAbove()->setSelected( true );
      newSelected = lvi->itemAbove();
    } else
      selectionChanged( 0 );

    delete lvi;

    if ( newSelected )
      mSensorView->ensureItemVisible( newSelected );
  }
}

void DancingBarsSettings::selectionChanged( Q3ListViewItem* lvi )
{
  bool state = ( lvi != 0 );

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
}


#include "DancingBarsSettings.moc"
