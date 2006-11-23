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
#include <kcolordialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>

#include <QtCore/QAbstractTableModel>
#include <QtCore/QList>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QImage>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QTreeView>

#include "FancyPlotterSettings.h"

FancyPlotterSettings::FancyPlotterSettings( QWidget* parent, bool locked )
  : KPageDialog( parent ), mModel( new SensorModel( this ) )
{
  setFaceType( Tabbed );
  setCaption( i18n( "Signal Plotter Settings" ) );
  setButtons( Ok | Cancel );
  setObjectName( "FancyPlotterSettings" );
  setModal( true );
  showButtonSeparator( true );

  QFrame *page = 0;
  QGridLayout *pageLayout = 0;
  QGridLayout *boxLayout = 0;
  QGroupBox *groupBox = 0;
  QLabel *label = 0;

  // Style page
  page = new QFrame();
  addPage( page, i18n( "General" ) );
  pageLayout = new QGridLayout( page );
  pageLayout->setSpacing( spacingHint() );
  pageLayout->setMargin( 0 );

  label = new QLabel( i18n( "Title:" ), page );
  pageLayout->addWidget( label, 0, 0 );

  mTitle = new KLineEdit( page );
  mTitle->setWhatsThis( i18n( "Enter the title of the display here." ) );
  pageLayout->addWidget( mTitle, 0, 1 );
  label->setBuddy( mTitle );

  pageLayout->setRowStretch( 1, 1 );

  // Scales page
  page = new QFrame();
  addPage( page, i18n( "Scales" ) );
  pageLayout = new QGridLayout( page );
  pageLayout->setSpacing( spacingHint() );
  pageLayout->setMargin( 0 );

  groupBox = new QGroupBox( i18n( "Vertical Scale" ), page );
  boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );
  boxLayout->setSpacing( spacingHint() );
  boxLayout->setColumnStretch( 2, 1 );

  mUseAutoRange = new QCheckBox( i18n( "Automatic range detection" ), groupBox );
  mUseAutoRange->setWhatsThis( i18n( "Check this box if you want the display range to adapt dynamically to the currently displayed values; if you do not check this, you have to specify the range you want in the fields below." ) );
  boxLayout->addWidget( mUseAutoRange, 0, 0, 1, 5 );

  label = new QLabel( i18n( "Minimum value:" ), groupBox );
  boxLayout->addWidget( label, 1, 0 );

  mMinValue = new KLineEdit( groupBox );
  mMinValue->setAlignment( Qt::AlignRight );
  mMinValue->setEnabled( false );
  mMinValue->setWhatsThis( i18n( "Enter the minimum value for the display here. If both values are 0, automatic range detection is enabled." ) );
  boxLayout->addWidget( mMinValue, 1, 1 );
  label->setBuddy( mMinValue );

  label = new QLabel( i18n( "Maximum value:" ), groupBox );
  boxLayout->addWidget( label, 1, 3 );

  mMaxValue = new KLineEdit( groupBox );
  mMaxValue->setAlignment( Qt::AlignRight );
  mMaxValue->setEnabled( false );
  mMaxValue->setWhatsThis( i18n( "Enter the maximum value for the display here. If both values are 0, automatic range detection is enabled." ) );
  boxLayout->addWidget( mMaxValue, 1, 4 );
  label->setBuddy( mMaxValue );

  pageLayout->addWidget( groupBox, 0, 0 );

  groupBox = new QGroupBox( i18n( "Horizontal Scale" ), page );
  boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );
  boxLayout->setSpacing( spacingHint() );
  boxLayout->setRowStretch( 1, 1 );

  mHorizontalScale = new KIntNumInput( 1, groupBox );
  mHorizontalScale->setMinimum( 1 );
  mHorizontalScale->setMaximum( 50 );
  boxLayout->addWidget( mHorizontalScale, 0, 0 );

  label = new QLabel( i18n( "pixels per time period" ), groupBox );
  boxLayout->addWidget( label, 0, 1 );

  pageLayout->addWidget( groupBox, 1, 0 );

  // Grid page
  page = new QFrame( );
  addPage( page, i18n( "Grid" ) );
  pageLayout = new QGridLayout( page );
  pageLayout->setSpacing( spacingHint() );
  pageLayout->setMargin( 0 );

  groupBox = new QGroupBox( i18n( "Lines" ), page );
  boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );
  boxLayout->setSpacing( spacingHint() );
  boxLayout->setColumnStretch( 1, 1 );

  mShowVerticalLines = new QCheckBox( i18n( "Vertical lines" ), groupBox );
  mShowVerticalLines->setWhatsThis( i18n( "Check this to activate the vertical lines if display is large enough." ) );
  boxLayout->addWidget( mShowVerticalLines, 0, 0 );

  label = new QLabel( i18n( "Distance:" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mVerticalLinesDistance = new KIntNumInput( 0, groupBox );
  mVerticalLinesDistance->setMinimum( 10 );
  mVerticalLinesDistance->setMaximum( 120 );
  mVerticalLinesDistance->setWhatsThis( i18n( "Enter the distance between two vertical lines here." ) );
  boxLayout->addWidget( mVerticalLinesDistance , 0, 3 );
  label->setBuddy( mVerticalLinesDistance );

  mVerticalLinesScroll = new QCheckBox( i18n( "Vertical lines scroll" ), groupBox );
  boxLayout->addWidget( mVerticalLinesScroll, 0, 4 );

  mShowHorizontalLines = new QCheckBox( i18n( "Horizontal lines" ), groupBox );
  mShowHorizontalLines->setWhatsThis( i18n( "Check this to enable horizontal lines if display is large enough." ) );
  boxLayout->addWidget( mShowHorizontalLines, 1, 0 );

  label = new QLabel( i18n( "Count:" ), groupBox );
  boxLayout->addWidget( label, 1, 2 );

  mHorizontalLinesCount = new KIntNumInput( 0, groupBox );
  mHorizontalLinesCount->setMinimum( 2 );
  mHorizontalLinesCount->setMaximum( 20 );
  mHorizontalLinesCount->setWhatsThis( i18n( "Enter the number of horizontal lines here." ) );
  boxLayout->addWidget( mHorizontalLinesCount , 1, 3 );
  label->setBuddy( mHorizontalLinesCount );

  boxLayout->setRowStretch( 2, 1 );

  pageLayout->addWidget( groupBox, 0, 0, 1, 2 );

  groupBox = new QGroupBox( i18n( "Text" ), page );
  boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );
  boxLayout->setSpacing( spacingHint() );
  boxLayout->setColumnStretch( 1, 1 );

  mShowLabels = new QCheckBox( i18n( "Labels" ), groupBox );
  mShowLabels->setWhatsThis( i18n( "Check this box if horizontal lines should be decorated with the values they mark." ) );
  boxLayout->addWidget( mShowLabels, 0, 0 );

  label = new QLabel( i18n( "Font size:" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mFontSize = new KIntNumInput( 9, groupBox );
  mFontSize->setMinimum( 5 );
  mFontSize->setMaximum( 24 );
  boxLayout->addWidget( mFontSize, 0, 3 );
  label->setBuddy( mFontSize );

  mShowTopBar = new QCheckBox( i18n( "Top bar" ), groupBox );
  mShowTopBar->setWhatsThis( i18n( "Check this to active the display title bar. This is probably only useful for applet displays. The bar is only visible if the display is large enough." ) );
  boxLayout->addWidget( mShowTopBar, 1, 0 );

  boxLayout->setRowStretch( 2, 1 );

  pageLayout->addWidget( groupBox, 1, 0 );

  groupBox = new QGroupBox( i18n( "Colors" ), page );
  boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );
  boxLayout->setSpacing( spacingHint() );

  label = new QLabel( i18n( "Font:" ), groupBox );
  boxLayout->addWidget( label, 0, 0 );
  mFontColor = new KColorButton( groupBox );
  boxLayout->addWidget( mFontColor, 0, 1 );
  label->setBuddy( mFontColor );


  label = new QLabel( i18n( "Vertical lines:" ), groupBox );
  boxLayout->addWidget( label, 1, 0 );
  mVerticalLinesColor = new KColorButton( groupBox );
  boxLayout->addWidget( mVerticalLinesColor, 1, 1 );
  label->setBuddy( mVerticalLinesColor );

  label = new QLabel( i18n( "Horizontal lines:" ), groupBox );
  boxLayout->addWidget( label, 2, 0 );

  mHorizontalLinesColor = new KColorButton( groupBox );
  boxLayout->addWidget( mHorizontalLinesColor, 2, 1 );
  label->setBuddy( mHorizontalLinesColor );

  label = new QLabel( i18n( "Background:" ), groupBox );
  boxLayout->addWidget( label, 3, 0 );

  mBackgroundColor = new KColorButton( groupBox );
  boxLayout->addWidget( mBackgroundColor, 3, 1 );
  label->setBuddy( mBackgroundColor );

  boxLayout->setRowStretch( 4, 1 );

  pageLayout->addWidget( groupBox, 1, 1 );

  pageLayout->setRowStretch( 2, 1 );

  // Sensors page
  page = new QFrame( );
  addPage( page, i18n( "Sensors" ) );
  pageLayout = new QGridLayout( page );
  pageLayout->setSpacing( spacingHint() );
  pageLayout->setMargin( 0 );
  pageLayout->setRowStretch( 2, 1 );
  pageLayout->setRowStretch( 5, 1 );

  mView = new QTreeView( page );
  mView->header()->setStretchLastSection( true );
  mView->setRootIsDecorated( false );
  mView->setItemsExpandable( false );
  mView->setModel( mModel );
  pageLayout->addWidget( mView, 0, 0, 6, 1 );

  mEditButton = new QPushButton( i18n( "Set Color..." ), page );
  mEditButton->setWhatsThis( i18n( "Push this button to configure the color of the sensor in the diagram." ) );
  pageLayout->addWidget( mEditButton, 0, 1 );

  mRemoveButton = 0;
  if ( !locked ) {
    mRemoveButton = new QPushButton( i18n( "Delete" ), page );
    mRemoveButton->setWhatsThis( i18n( "Push this button to delete the sensor." ) );
    pageLayout->addWidget( mRemoveButton, 1, 1 );

    connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeSensor() ) );
  }

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

  connect( mEditButton, SIGNAL( clicked() ), SLOT( editSensor() ) );

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
  mMinValue->setEnabled( !value );
  mMaxValue->setEnabled( !value );
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
  mVerticalLinesDistance->setEnabled(  value );
  mVerticalLinesScroll->setEnabled( value );
  mVerticalLinesColor->setEnabled( value );
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
  mHorizontalLinesCount->setEnabled( value );
  mHorizontalLinesColor->setEnabled( value );
  mShowLabels->setEnabled( value );

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
void FancyPlotterSettings::setFontColor( const QColor &color )
{
  mFontColor->setColor( color );
}

QColor FancyPlotterSettings::fontColor() const
{
  return mFontColor->color();
}
void FancyPlotterSettings::setBackgroundColor( const QColor &color )
{
  mBackgroundColor->setColor( color );
}

QColor FancyPlotterSettings::backgroundColor() const
{
  return mBackgroundColor->color();
}

void FancyPlotterSettings::setSensors( const SensorModelEntry::List &list )
{
  mModel->setSensors( list );

  mView->selectionModel()->setCurrentIndex( mModel->index( 0, 0 ), QItemSelectionModel::SelectCurrent |
                                                                   QItemSelectionModel::Rows );
}

SensorModelEntry::List FancyPlotterSettings::sensors() const
{
  return mModel->sensors();
}

void FancyPlotterSettings::editSensor()
{
  if ( !mView->selectionModel() )
    return;

  const QModelIndex index = mView->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return;

  SensorModelEntry sensor = mModel->sensor( index );

  QColor color = sensor.color();
  int result = KColorDialog::getColor( color, parentWidget() );
  if ( result == KColorDialog::Accepted ) {
    sensor.setColor( color );
    mModel->setSensor( sensor, index );
  }
}

void FancyPlotterSettings::removeSensor()
{
  if ( !mView->selectionModel() )
    return;

  const QModelIndex index = mView->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return;

  mModel->removeSensor( index );
}

#include "FancyPlotterSettings.moc"
