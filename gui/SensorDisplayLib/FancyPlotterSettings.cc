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

    $Id$
*/

#include <kaccelmanager.h>
#include <kcolorbutton.h>
#include <kcolordialog.h>
#include <klineedit.h>
#include <klistview.h>
#include <klocale.h>
#include <knuminput.h>

#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>

#include "FancyPlotterSettings.h"

FancyPlotterSettings::FancyPlotterSettings( QWidget* parent, const char* name )
  : KDialogBase( Tabbed, i18n( "Signal Plotter Settings" ), Ok | Apply | Cancel,
                 Ok, parent, name, true, true )
{
  QFrame *page = 0;
  QGridLayout *pageLayout = 0;
  QGridLayout *boxLayout = 0;
  QGroupBox *groupBox = 0;
  QLabel *label = 0;

  // Style page
  page = addPage( i18n( "Style" ) );
  pageLayout = new QGridLayout( page, 3, 2, 0, spacingHint() );

  label = new QLabel( i18n( "Title:" ), page );
  pageLayout->addWidget( label, 0, 0 );

  mTitle = new KLineEdit( page );
  QWhatsThis::add( mTitle, i18n( "Enter the title of the display here." ) );
  pageLayout->addWidget( mTitle, 0, 1 );
  label->setBuddy( mTitle );

  QButtonGroup *buttonBox = new QButtonGroup( 2, Qt::Vertical,
                                              i18n( "Graph Drawing Style" ), page );

  mUsePolygonStyle = new QRadioButton( i18n( "Basic polygons" ), buttonBox );
  mUsePolygonStyle->setChecked( true );
  mUseOriginalStyle = new QRadioButton( i18n( "Original - single line per data point" ), buttonBox );

  pageLayout->addMultiCellWidget( buttonBox, 1, 1, 0, 1 );

  // Scales page
  page = addPage( i18n( "Scales" ) );
  pageLayout = new QGridLayout( page, 2, 1, 0, spacingHint() );

  groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "Vertical Scale" ), page );
  boxLayout = new QGridLayout( groupBox->layout(), 2, 5, spacingHint() );
  boxLayout->setColStretch( 2, 1 );

  mUseAutoRange = new QCheckBox( i18n( "Automatic range detection" ), groupBox );
  QWhatsThis::add( mUseAutoRange, i18n( "Check this box if you want the display range to adapt dynamically to the currently displayed values. If you don't check this, you have to specify the range you want in the fields below." ) );
  boxLayout->addMultiCellWidget( mUseAutoRange, 0, 0, 0, 4 );

  label = new QLabel( i18n( "Minimum value:" ), groupBox );
  boxLayout->addWidget( label, 1, 0 );

  mMinValue = new KLineEdit( groupBox );
  mMinValue->setAlignment( AlignRight );
  mMinValue->setEnabled( false );
  QWhatsThis::add( mMinValue, i18n( "Enter the minimum value for the display here. If both values are 0 automatic range detection is enabled." ) );
  boxLayout->addWidget( mMinValue, 1, 1 );
  label->setBuddy( mMinValue );

  label = new QLabel( i18n( "Maximum value:" ), groupBox );
  boxLayout->addWidget( label, 1, 3 );

  mMaxValue = new KLineEdit( groupBox );
  mMaxValue->setAlignment( AlignRight );
  mMaxValue->setEnabled( false );
  QWhatsThis::add( mMaxValue, i18n( "Enter the maximum value for the display here. If both values are 0 automatic range detection is enabled." ) );
  boxLayout->addWidget( mMaxValue, 1, 4 );
  label->setBuddy( mMaxValue );

  pageLayout->addWidget( groupBox, 0, 0 );

  groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "Horizontal Scale" ), page );
  boxLayout = new QGridLayout( groupBox->layout(), 2, 2, spacingHint() );
  boxLayout->setRowStretch( 1, 1 );

  mHorizontalScale = new KIntNumInput( 1, groupBox );
  mHorizontalScale->setMinValue( 1 );
  mHorizontalScale->setMaxValue( 50 );
  boxLayout->addWidget( mHorizontalScale, 0, 0 );

  label = new QLabel( i18n( "pixel(s) per time period" ), groupBox );
  boxLayout->addWidget( label, 0, 1 );

  pageLayout->addWidget( groupBox, 1, 0 );

  // Grid page
  page = addPage( i18n( "Grid" ) );
  pageLayout = new QGridLayout( page, 3, 2, 0, spacingHint() );

  groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "Lines" ), page );
  boxLayout = new QGridLayout( groupBox->layout(), 2, 5, spacingHint() );
  boxLayout->setColStretch( 1, 1 );

  mShowVerticalLines = new QCheckBox( i18n( "Vertical lines" ), groupBox );
  QWhatsThis::add( mShowVerticalLines, i18n( "Check this to activate the vertical lines if display is large enough." ) );
  boxLayout->addWidget( mShowVerticalLines, 0, 0 );

  label = new QLabel( i18n( "Distance:" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mVerticalLinesDistance = new KIntNumInput( 0, groupBox );
  mVerticalLinesDistance->setMinValue( 10 );
  mVerticalLinesDistance->setMaxValue( 120 );
  QWhatsThis::add( mVerticalLinesDistance, i18n( "Enter the distance between two vertical lines here." ) );
  boxLayout->addWidget( mVerticalLinesDistance , 0, 3 );
  label->setBuddy( mVerticalLinesDistance );

  mVerticalLinesScroll = new QCheckBox( i18n( "Vertical lines scroll" ), groupBox );
  boxLayout->addWidget( mVerticalLinesScroll, 0, 4 );

  mShowHorizontalLines = new QCheckBox( i18n( "Horizontal lines" ), groupBox );
  QWhatsThis::add( mShowHorizontalLines, i18n( "Check this to enable horizontal lines if display is large enough." ) );
  boxLayout->addWidget( mShowHorizontalLines, 1, 0 );

  label = new QLabel( i18n( "Count:" ), groupBox );
  boxLayout->addWidget( label, 1, 2 );

  mHorizontalLinesCount = new KIntNumInput( 0, groupBox );
  mHorizontalLinesCount->setMinValue( 2 );
  mHorizontalLinesCount->setMaxValue( 20 );
  QWhatsThis::add( mHorizontalLinesCount, i18n( "Enter the number of horizontal lines here." ) );
  boxLayout->addWidget( mHorizontalLinesCount , 1, 3 );
  label->setBuddy( mHorizontalLinesCount );

  boxLayout->setRowStretch( 2, 1 );

  pageLayout->addMultiCellWidget( groupBox, 0, 0, 0, 1 );

  groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "Text" ), page );
  boxLayout = new QGridLayout( groupBox->layout(), 3, 4, spacingHint() );
  boxLayout->setColStretch( 1, 1 );

  mShowLabels = new QCheckBox( i18n( "Labels" ), groupBox );
  QWhatsThis::add( mShowLabels, i18n( "Check this box if horizontal lines should be decorated with the values they mark." ) );
  boxLayout->addWidget( mShowLabels, 0, 0 );

  label = new QLabel( i18n( "Font size:" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mFontSize = new KIntNumInput( 9, groupBox );
  mFontSize->setMinValue( 5 );
  mFontSize->setMaxValue( 24 );
  boxLayout->addWidget( mFontSize, 0, 3 );
  label->setBuddy( mFontSize );

  mShowTopBar = new QCheckBox( i18n( "Top bar" ), groupBox );
  QWhatsThis::add( mShowTopBar, i18n( "Check this to active the display title bar. This is probably only useful for applet displays. The bar is only visible if the display is large enough." ) );
  boxLayout->addWidget( mShowTopBar, 1, 0 );

  boxLayout->setRowStretch( 2, 1 );

  pageLayout->addWidget( groupBox, 1, 0 );

  groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "Colors" ), page );
  boxLayout = new QGridLayout( groupBox->layout(), 4, 2, spacingHint() );

  label = new QLabel( i18n( "Vertical lines:" ), groupBox );
  boxLayout->addWidget( label, 0, 0 );

  mVerticalLinesColor = new KColorButton( groupBox );
  boxLayout->addWidget( mVerticalLinesColor, 0, 1 );
  label->setBuddy( mVerticalLinesColor );

  label = new QLabel( i18n( "Horizontal lines:" ), groupBox );
  boxLayout->addWidget( label, 1, 0 );

  mHorizontalLinesColor = new KColorButton( groupBox );
  boxLayout->addWidget( mHorizontalLinesColor, 1, 1 );
  label->setBuddy( mHorizontalLinesColor );

  label = new QLabel( i18n( "Background:" ), groupBox );
  boxLayout->addWidget( label, 2, 0 );

  mBackgroundColor = new KColorButton( groupBox );
  boxLayout->addWidget( mBackgroundColor, 2, 1 );
  label->setBuddy( mBackgroundColor );

  boxLayout->setRowStretch( 3, 1 );

  pageLayout->addWidget( groupBox, 1, 1 );

  pageLayout->setRowStretch( 2, 1 );

  // Sensors page
  page = addPage( i18n( "Sensors" ) );
  pageLayout = new QGridLayout( page, 6, 2, 0, spacingHint() );
  pageLayout->setRowStretch( 2, 1 );
  pageLayout->setRowStretch( 5, 1 );

  mSensorView = new KListView( page );
  mSensorView->addColumn( i18n( "#" ) );
  mSensorView->addColumn( i18n( "Host" ) );
  mSensorView->addColumn( i18n( "Sensor" ) );
  mSensorView->addColumn( i18n( "Unit" ) );
  mSensorView->addColumn( i18n( "Status" ) );
  mSensorView->setAllColumnsShowFocus( true );
  pageLayout->addMultiCellWidget( mSensorView, 0, 5, 0, 0 );

  mEditButton = new QPushButton( i18n( "Set Color..." ), page );
  mEditButton->setEnabled( false );
  QWhatsThis::add( mEditButton, i18n( "Push this button to configure the color of the sensor in the diagram." ) );
  pageLayout->addWidget( mEditButton, 0, 1 );

  mRemoveButton = new QPushButton( i18n( "Delete" ), page );
  mRemoveButton->setEnabled( false );
  QWhatsThis::add( mRemoveButton, i18n( "Push this button to delete the sensor." ) );
  pageLayout->addWidget( mRemoveButton, 1, 1 );

  mMoveUpButton = new QPushButton( i18n( "Move Up" ), page );
  mMoveUpButton->setEnabled( false );
  pageLayout->addWidget( mMoveUpButton, 3, 1 );

  mMoveDownButton = new QPushButton( i18n( "Move Down" ), page );
  mMoveDownButton->setEnabled( false );
  pageLayout->addWidget( mMoveDownButton, 4, 1 );

  connect( mUseAutoRange, SIGNAL( toggled( bool ) ), mMinValue,
           SLOT( setDisabled( bool ) ) );
  connect( mUseAutoRange, SIGNAL( toggled( bool ) ), mMaxValue,
           SLOT( setDisabled( bool ) ) );
  connect( mShowVerticalLines, SIGNAL( toggled( bool ) ), mVerticalLinesDistance,
           SLOT( setEnabled( bool ) ) );
  connect( mShowVerticalLines, SIGNAL( toggled( bool ) ), mVerticalLinesScroll,
           SLOT( setEnabled( bool ) ) );
  connect( mShowVerticalLines, SIGNAL( toggled( bool ) ), mVerticalLinesColor,
           SLOT( setEnabled( bool ) ) );
  connect( mShowHorizontalLines, SIGNAL( toggled( bool ) ), mHorizontalLinesCount,
           SLOT( setEnabled( bool ) ) );
  connect( mShowHorizontalLines, SIGNAL( toggled( bool ) ), mHorizontalLinesColor,
           SLOT( setEnabled( bool ) ) );
  connect( mShowHorizontalLines, SIGNAL( toggled( bool ) ), mShowLabels,
           SLOT( setEnabled( bool ) ) );
  connect( mSensorView, SIGNAL( selectionChanged( QListViewItem* ) ),
           SLOT( selectionChanged( QListViewItem* ) ) );

  connect( mEditButton, SIGNAL( clicked() ), SLOT( editSensor() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeSensor() ) );
  connect( mMoveUpButton, SIGNAL( clicked() ), SLOT( moveUpSensor() ) );
  connect( mMoveDownButton, SIGNAL( clicked() ), SLOT( moveDownSensor() ) );
  connect ( mSensorView, SIGNAL( doubleClicked( QListViewItem *, const QPoint &, int )), SLOT(editSensor()));

  KAcceleratorManager::manage( this );
}

FancyPlotterSettings::~FancyPlotterSettings()
{
}

void FancyPlotterSettings::setTitle( const QString &title )
{
  mTitle->setText( title );
}

QString FancyPlotterSettings::title() const
{
  return mTitle->text();
}

void FancyPlotterSettings::setUseAutoRange( bool value )
{
  mUseAutoRange->setChecked( value );
}

bool FancyPlotterSettings::useAutoRange() const
{
  return mUseAutoRange->isChecked();
}

void FancyPlotterSettings::setMinValue( double min )
{
  mMinValue->setText( QString::number( min ) );
}

double FancyPlotterSettings::minValue() const
{
  return mMinValue->text().toDouble();
}

void FancyPlotterSettings::setMaxValue( double max )
{
  mMaxValue->setText( QString::number( max ) );
}

double FancyPlotterSettings::maxValue() const
{
  return mMaxValue->text().toDouble();
}

void FancyPlotterSettings::setUsePolygonStyle( bool value )
{
  if ( value )
    mUsePolygonStyle->setChecked( true );
  else
    mUseOriginalStyle->setChecked( true );
}

bool FancyPlotterSettings::usePolygonStyle() const
{
  return mUsePolygonStyle->isChecked();
}

void FancyPlotterSettings::setHorizontalScale( int scale )
{
  mHorizontalScale->setValue( scale );
}

int FancyPlotterSettings::horizontalScale() const
{
  return mHorizontalScale->value();
}

void FancyPlotterSettings::setShowVerticalLines( bool value )
{
  mShowVerticalLines->setChecked( value );
}

bool FancyPlotterSettings::showVerticalLines() const
{
  return mShowVerticalLines->isChecked();
}

void FancyPlotterSettings::setVerticalLinesColor( const QColor &color )
{
  mVerticalLinesColor->setColor( color );
}

QColor FancyPlotterSettings::verticalLinesColor() const
{
  return mVerticalLinesColor->color();
}

void FancyPlotterSettings::setVerticalLinesDistance( int distance )
{
  mVerticalLinesDistance->setValue( distance );
}

int FancyPlotterSettings::verticalLinesDistance() const
{
  return mVerticalLinesDistance->value();
}

void FancyPlotterSettings::setVerticalLinesScroll( bool value )
{
  mVerticalLinesScroll->setChecked( value );
}

bool FancyPlotterSettings::verticalLinesScroll() const
{
  return mVerticalLinesScroll->isChecked();
}

void FancyPlotterSettings::setShowHorizontalLines( bool value )
{
  mShowHorizontalLines->setChecked( value );
}

bool FancyPlotterSettings::showHorizontalLines() const
{
  return mShowHorizontalLines->isChecked();
}

void FancyPlotterSettings::setHorizontalLinesColor( const QColor &color )
{
  mHorizontalLinesColor->setColor( color );
}

QColor FancyPlotterSettings::horizontalLinesColor() const
{
  return mHorizontalLinesColor->color();
}

void FancyPlotterSettings::setHorizontalLinesCount( int count )
{
  mHorizontalLinesCount->setValue( count );
}

int FancyPlotterSettings::horizontalLinesCount() const
{
  return mHorizontalLinesCount->value();
}

void FancyPlotterSettings::setShowLabels( bool value )
{
  mShowLabels->setChecked( value );
}

bool FancyPlotterSettings::showLabels() const
{
  return mShowLabels->isChecked();
}

void FancyPlotterSettings::setShowTopBar( bool value )
{
  mShowTopBar->setChecked( value );
}

bool FancyPlotterSettings::showTopBar() const
{
  return mShowTopBar->isChecked();
}

void FancyPlotterSettings::setFontSize( int size )
{
  mFontSize->setValue( size );
}

int FancyPlotterSettings::fontSize() const
{
  return mFontSize->value();
}

void FancyPlotterSettings::setBackgroundColor( const QColor &color )
{
  mBackgroundColor->setColor( color );
}

QColor FancyPlotterSettings::backgroundColor() const
{
  return mBackgroundColor->color();
}

void FancyPlotterSettings::setSensors( const QValueList< QStringList > &list )
{
  mSensorView->clear();

  QValueList< QStringList >::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    QListViewItem* lvi = new QListViewItem( mSensorView,
                                            (*it)[ 0 ],   // id
                                            (*it)[ 1 ],   // host name
                                            (*it)[ 2 ],   // sensor name
                                            (*it)[ 3 ],   // unit
                                            (*it)[ 4 ] ); // status
    QPixmap pm( 12, 12 );
    pm.fill( QColor( (*it)[ 5 ] ) );
    lvi->setPixmap( 2, pm );
    mSensorView->insertItem( lvi );
  }
}

QValueList< QStringList > FancyPlotterSettings::sensors() const
{
  QValueList< QStringList > list;

  QListViewItemIterator it( mSensorView );

  for ( ; it.current(); ++it ) {
    QStringList entry;
    entry << it.current()->text( 0 );
    entry << it.current()->text( 1 );
    entry << it.current()->text( 2 );
    entry << it.current()->text( 3 );
    entry << it.current()->text( 4 );
    QRgb rgb = it.current()->pixmap( 2 )->convertToImage().pixel( 1, 1 );
    QColor color( qRed( rgb ), qGreen( rgb ), qBlue( rgb ) );
    entry << ( color.name() );

    list.append( entry );
  }

  return list;
}

void FancyPlotterSettings::editSensor()
{
  QListViewItem* lvi = mSensorView->currentItem();

  if ( !lvi )
    return;

  QColor color = lvi->pixmap( 2 )->convertToImage().pixel( 1, 1 );
  int result = KColorDialog::getColor( color, parentWidget() );
  if ( result == KColorDialog::Accepted ) {
    QPixmap newPm( 12, 12 );
    newPm.fill( color );
    lvi->setPixmap( 2, newPm );
  }
}

void FancyPlotterSettings::removeSensor()
{
  QListViewItem* lvi = mSensorView->currentItem();

  if ( lvi ) {
    /* Before we delete the currently selected item, we determine a
     * new item to be selected. That way we can ensure that multiple
     * items can be deleted without forcing the user to select a new
     * item between the deletes. If all items are deleted, the buttons
     * are disabled again. */
    QListViewItem* newSelected = 0;
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

void FancyPlotterSettings::moveUpSensor()
{
  if ( mSensorView->currentItem() != 0 ) {
    QListViewItem* item = mSensorView->currentItem()->itemAbove();
    if ( item ) {
      if ( item->itemAbove() )
        mSensorView->currentItem()->moveItem( item->itemAbove() );
      else
        item->moveItem( mSensorView->currentItem() );
    }

    // Re-calculate the "sensor number" field
    item = mSensorView->firstChild();
    for ( uint count = 1; item; item = item->itemBelow(), count++ )
      item->setText( 0, QString( "%1" ).arg( count ) );

    selectionChanged( mSensorView->currentItem() );
  }
}

void FancyPlotterSettings::moveDownSensor()
{
  if ( mSensorView->currentItem() != 0 ) {
    if ( mSensorView->currentItem()->itemBelow() )
      mSensorView->currentItem()->moveItem( mSensorView->currentItem()->itemBelow() );

    // Re-calculate the "sensor number" field
    QListViewItem* item = mSensorView->firstChild();
    for ( uint count = 1; item; item = item->itemBelow(), count++ )
      item->setText( 0, QString( "%1" ).arg( count ) );

    selectionChanged( mSensorView->currentItem() );
  }
}

void FancyPlotterSettings::selectionChanged( QListViewItem *item )
{
  bool state = ( item != 0 );

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
  mMoveUpButton->setEnabled( state && item->itemAbove() );
  mMoveDownButton->setEnabled( state && item->itemBelow() );
}

#include "FancyPlotterSettings.moc"
