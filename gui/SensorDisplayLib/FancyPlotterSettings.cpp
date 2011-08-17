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
#include <kcolordialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>

#include <QtCore/QAbstractTableModel>
#include <QtCore/QList>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFormLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QImage>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QTreeView>

#include <limits>

#include "FancyPlotterSettings.h"

FancyPlotterSettings::FancyPlotterSettings( QWidget* parent, bool locked )
  : KPageDialog( parent ), mModel( new SensorModel( this ) )
{
  setFaceType( Tabbed );
  setCaption( i18n( "Plotter Settings" ) );
  setButtons( Ok | Apply | Cancel );
  setObjectName( "FancyPlotterSettings" );
  setModal( false );

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

  mStackBeams = new QCheckBox( i18n("Stack the beams on top of each other"), page);
  mStackBeams->setWhatsThis( i18n("The beams are stacked on top of each other, and the area is drawn filled in. So if one beam has a value of 2 and another beam has a value of 3, the first beam will be drawn at value 2 and the other beam drawn at 2+3=5.") );
  pageLayout->addWidget( mStackBeams, 1, 0,1,2);

  pageLayout->setRowStretch( 2, 1 );

  // Scales page
  page = new QFrame();
  addPage( page, i18n( "Scales" ) );
  pageLayout = new QGridLayout( page );
  pageLayout->setSpacing( spacingHint() );
  pageLayout->setMargin( 0 );

  groupBox = new QGroupBox( i18n( "Vertical scale" ), page );
  boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );
  boxLayout->setSpacing( spacingHint() );
  boxLayout->setColumnStretch( 2, 1 );

  mManualRange = new QCheckBox( i18n( "Specify graph range:" ), groupBox );
  mManualRange->setWhatsThis( i18n( "Check this box if you want the display range to adapt dynamically to the currently displayed values; if you do not check this, you have to specify the range you want in the fields below." ) );
  mManualRange->setChecked(true);
  boxLayout->addWidget( mManualRange, 0, 0, 1, 5 );

  mMinValueLabel = new QLabel( i18n( "Minimum value:" ), groupBox );
  boxLayout->addWidget( mMinValueLabel, 1, 0 );

  mMinValue = new QDoubleSpinBox( groupBox );
  mMinValue->setMaximum( std::numeric_limits<long long>::max());
  mMinValue->setMinimum( std::numeric_limits<long long>::min());
  mMinValue->setWhatsThis( i18n( "Enter the minimum value for the display here." ) );
  mMinValue->setSingleStep(10);
  boxLayout->addWidget( mMinValue, 1, 1 );
  mMinValueLabel->setBuddy( mMinValue );

  mMaxValueLabel = new QLabel( i18n( "Maximum value:" ), groupBox );
  boxLayout->addWidget( mMaxValueLabel, 1, 3 );

  mMaxValue = new QDoubleSpinBox( groupBox);
  mMaxValue->setMaximum( std::numeric_limits<long long>::max());
  mMaxValue->setMinimum( std::numeric_limits<long long>::min());
  mMaxValue->setWhatsThis( i18n( "Enter the soft maximum value for the display here. The upper range will not be reduced below this value, but will still go above this number for values above this value." ) );
  mMaxValue->setSingleStep(10);
  boxLayout->addWidget( mMaxValue, 1, 4 );
  mMaxValueLabel->setBuddy( mMaxValue );

  pageLayout->addWidget( groupBox, 0, 0 );

  groupBox = new QGroupBox( i18n( "Horizontal scale" ), page );
  QFormLayout *formLayout = new QFormLayout(groupBox);

  mHorizontalScale = new KIntNumInput( 1, groupBox );
  mHorizontalScale->setMinimum( 1 );
  mHorizontalScale->setMaximum( 50 );

  formLayout->addRow( i18n("Pixels per time period:"), mHorizontalScale );


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
  boxLayout->addWidget( mVerticalLinesScroll, 1, 0, 1, -1 );

  mShowHorizontalLines = new QCheckBox( i18n( "Horizontal lines" ), groupBox );
  mShowHorizontalLines->setWhatsThis( i18n( "Check this to enable horizontal lines if display is large enough." ) );
  boxLayout->addWidget( mShowHorizontalLines, 2, 0, 1, -1 );

  pageLayout->addWidget( groupBox, 0, 0, 1, 2 );

  groupBox = new QGroupBox( i18n( "Text" ), page );
  boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );
  boxLayout->setSpacing( spacingHint() );
  boxLayout->setColumnStretch( 1, 1 );

  mShowAxis = new QCheckBox( i18n( "Show axis labels" ), groupBox );
  mShowAxis->setWhatsThis( i18n( "Check this box if horizontal lines should be decorated with the values they mark." ) );
  boxLayout->addWidget( mShowAxis, 0, 0, 1, -1 );

  label = new QLabel( i18n( "Font size:" ), groupBox );
  boxLayout->addWidget( label, 1, 0 );

  mFontSize = new KIntNumInput( 8, groupBox );
  mFontSize->setMinimum( 1 );
  mFontSize->setMaximum( 1000 );
  boxLayout->addWidget( mFontSize, 1, 1 );
  label->setBuddy( mFontSize );

  pageLayout->addWidget( groupBox, 1, 0 );

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
  mView->header()->setStretchLastSection( false );
  mView->setRootIsDecorated( false );
  mView->setItemsExpandable( false );
  mView->setModel( mModel );
  mView->header()->setResizeMode(QHeaderView::ResizeToContents);
  bool hideFirstColumn = true;
  for(int i = 0; i < mModel->rowCount(); i++)
    if(mModel->data(mModel->index(i, 0)) != "localhost") {
      hideFirstColumn = false;
      break;
    }
  if(hideFirstColumn)
    mView->hideColumn(0);

  pageLayout->addWidget( mView, 0, 0, 6, 1 );
  connect(mView,SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editSensor()));

  mEditButton = new QPushButton( i18n( "Set Color..." ), page );
  mEditButton->setWhatsThis( i18n( "Push this button to configure the color of the sensor in the diagram." ) );
  pageLayout->addWidget( mEditButton, 0, 1 );

  mRemoveButton = 0;
  mMoveUpButton = 0;
  mMoveDownButton = 0;
  if ( !locked ) {
    mRemoveButton = new QPushButton( i18n( "Delete" ), page );
    mRemoveButton->setWhatsThis( i18n( "Push this button to delete the sensor." ) );
    pageLayout->addWidget( mRemoveButton, 1, 1 );
    connect( mRemoveButton, SIGNAL(clicked()), SLOT(removeSensor()) );

    mMoveUpButton = new QPushButton( i18n( "Move Up" ), page );
    mMoveUpButton->setEnabled( false );
    pageLayout->addWidget( mMoveUpButton, 2, 1 );
    connect( mMoveUpButton, SIGNAL(clicked()), SLOT(moveUpSensor()) );

    mMoveDownButton = new QPushButton( i18n( "Move Down" ), page );
    mMoveDownButton->setEnabled( false );
    pageLayout->addWidget( mMoveDownButton, 3, 1 );
    connect( mMoveDownButton, SIGNAL(clicked()), SLOT(moveDownSensor()) );

    connect(mView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChanged(QModelIndex)));

  }

  connect( mManualRange, SIGNAL(toggled(bool)), mMinValue,
           SLOT(setEnabled(bool)) );
  connect( mManualRange, SIGNAL(toggled(bool)), mMaxValue,
           SLOT(setEnabled(bool)) );
  connect( mManualRange, SIGNAL(toggled(bool)), mMinValueLabel,
           SLOT(setEnabled(bool)) );
  connect( mManualRange, SIGNAL(toggled(bool)), mMaxValueLabel,
           SLOT(setEnabled(bool)) );

  connect( mShowVerticalLines, SIGNAL(toggled(bool)), mVerticalLinesDistance,
           SLOT(setEnabled(bool)) );
  connect( mShowVerticalLines, SIGNAL(toggled(bool)), mVerticalLinesScroll,
           SLOT(setEnabled(bool)) );

  connect( mEditButton, SIGNAL(clicked()), SLOT(editSensor()) );

  KAcceleratorManager::manage( this );
}

FancyPlotterSettings::~FancyPlotterSettings()
{
}

void FancyPlotterSettings::setStackBeams( bool stack )
{
  mStackBeams->setChecked(stack);
}
bool FancyPlotterSettings::stackBeams() const {
  return mStackBeams->isChecked();
}
void FancyPlotterSettings::moveUpSensor()
{
  mModel->moveUpSensor(mView->selectionModel()->currentIndex());
  selectionChanged(mView->selectionModel()->currentIndex());
}
void FancyPlotterSettings::moveDownSensor()
{
  mModel->moveDownSensor(mView->selectionModel()->currentIndex());
  selectionChanged(mView->selectionModel()->currentIndex());
}
void FancyPlotterSettings::selectionChanged(const QModelIndex &newCurrent)
{
  mMoveUpButton->setEnabled(newCurrent.isValid() && newCurrent.row() > 0);
  mMoveDownButton->setEnabled(newCurrent.isValid() && newCurrent.row() < mModel->rowCount() -1 );
  mEditButton->setEnabled(newCurrent.isValid());
  mRemoveButton->setEnabled(newCurrent.isValid());
}

void FancyPlotterSettings::setTitle( const QString &title )
{
  mTitle->setText( title );
}

QString FancyPlotterSettings::title() const
{
  return mTitle->text();
}

void FancyPlotterSettings::setRangeUnits( const QString & units )
{
  mMinValue->setSuffix(' ' + units);
  mMaxValue->setSuffix(' ' + units);
}

void FancyPlotterSettings::setUseManualRange( bool value )
{
  mManualRange->setChecked( value );
}

bool FancyPlotterSettings::useManualRange() const
{
  return mManualRange->isChecked();
}

void FancyPlotterSettings::setMinValue( double min )
{
  mMinValue->setValue( min );
}

double FancyPlotterSettings::minValue() const
{
  return mMinValue->value();
}

void FancyPlotterSettings::setMaxValue( double max )
{
  mMaxValue->setValue( max );
}

void FancyPlotterSettings::setHasIntegerRange( bool hasIntegerRange )
{
    mMaxValue->setDecimals( hasIntegerRange?0:2 );
    mMinValue->setDecimals( hasIntegerRange?0:2 );
}
double FancyPlotterSettings::maxValue() const
{
  return mMaxValue->value();
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
}

bool FancyPlotterSettings::showVerticalLines() const
{
  return mShowVerticalLines->isChecked();
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

void FancyPlotterSettings::setShowAxis( bool value )
{
  mShowAxis->setChecked( value );
}

bool FancyPlotterSettings::showAxis() const
{
  return mShowAxis->isChecked();
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

  KColorDialog dialog(this, true);
  connect(&dialog, SIGNAL(colorSelected(QColor)), this, SLOT(setColorForSelectedItem(QColor)));
  QColor color = sensor.color();
  dialog.setColor(color);
  int result = dialog.exec();

  if ( result == KColorDialog::Accepted )
    sensor.setColor( dialog.color() );
  //If it's not accepted, make sure we set the color back to how it was
  mModel->setSensor( sensor, index );
}

void FancyPlotterSettings::setColorForSelectedItem(const QColor &color)
{
    const QModelIndex index = mView->selectionModel()->currentIndex();
    if ( !index.isValid() )
        return;

    SensorModelEntry sensor = mModel->sensor( index );

    sensor.setColor( color );
    mModel->setSensor( sensor, index );
}

void FancyPlotterSettings::removeSensor()
{
  if ( !mView->selectionModel() )
    return;

  const QModelIndex index = mView->selectionModel()->currentIndex();
  if ( !index.isValid() )
    return;
  mModel->removeSensor( index );
  selectionChanged( mView->selectionModel()->currentIndex() );
}

void FancyPlotterSettings::clearDeleted()
{
  mModel->clearDeleted();
}

QList<int> FancyPlotterSettings::deleted() const
{
  return mModel->deleted();
}

QList<int> FancyPlotterSettings::order() const
{
  return mModel->order();
}

void FancyPlotterSettings::resetOrder()
{
  mModel->resetOrder();
}



#include "FancyPlotterSettings.moc"
