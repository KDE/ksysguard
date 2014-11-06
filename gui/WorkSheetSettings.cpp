/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy 
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <kacceleratormanager.h>
#include <klineedit.h>
#include <KLocalizedString>
#include <knuminput.h>

#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

#include "WorkSheetSettings.h"

WorkSheetSettings::WorkSheetSettings( QWidget* parent, bool locked )
  : QDialog( parent )
{
  setObjectName( "WorkSheetSettings" );
  setModal( true );
  setWindowTitle( i18n( "Tab Properties" ) );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QWidget *page = new QWidget( this );
  mainLayout->addWidget(page);
  mainLayout->addWidget(buttonBox);

  QVBoxLayout *topLayout = new QVBoxLayout( page );
  topLayout->setMargin( 0 );
  //PORT QT5 topLayout->setSpacing( spacingHint() );

  QGroupBox *group = new QGroupBox( i18n( "Title" ), page );

  QGridLayout *groupLayout = new QGridLayout;
  group->setLayout( groupLayout );
  groupLayout->setAlignment( Qt::AlignTop );

  mSheetTitle = new KLineEdit( group );
  groupLayout->addWidget( mSheetTitle, 0, 0 );

  topLayout->addWidget( group );

  group = new QGroupBox( i18n( "Properties" ), page );

  groupLayout = new QGridLayout;
  group->setLayout( groupLayout );
  groupLayout->setAlignment( Qt::AlignTop );

  int row_num = -1;
  QLabel *label;
  if ( !locked ) {
    label = new QLabel( i18n( "Rows:" ), group );
    groupLayout->addWidget( label, ++row_num, 0 );

    mRows = new KIntNumInput( 3, group );
    mRows->setMaximum( 42 );
    mRows->setMinimum( 1 );
    groupLayout->addWidget( mRows, row_num, 1 );
    label->setBuddy( mRows );

    label = new QLabel( i18n( "Columns:" ), group );
    groupLayout->addWidget( label, ++row_num, 0 );

    mColumns = new KIntNumInput( 1, group );
    mColumns->setMaximum( 42 );
    mColumns->setMinimum( 1 );
    groupLayout->addWidget( mColumns, 1, 1 );
    label->setBuddy( mColumns );
    mRows->setWhatsThis( i18n( "Enter the number of rows the sheet should have." ) );
    mColumns->setWhatsThis( i18n( "Enter the number of columns the sheet should have." ) );
  }
  label = new QLabel( i18n( "Update interval:" ), group );
  groupLayout->addWidget( label, ++row_num, 0 );

  mInterval = new KDoubleNumInput( 0.00/*minimum*/, 1000.0/*maximum*/, 0.5/*default*/, group/*parent*/, 0.5/*stepsize*/, 2/*precision*/ );
  mInterval->setSuffix( i18n( " sec" ) );
  groupLayout->addWidget( mInterval, row_num, 1 );
  label->setBuddy( mInterval );

  topLayout->addWidget( group );

  mInterval->setWhatsThis( i18n( "All displays of the sheet are updated at the rate specified here." ) );
  mSheetTitle->setToolTip( i18n( "Enter the title of the worksheet here." ) );

  KAcceleratorManager::manage( page );

  mSheetTitle->setFocus();

//  resize( QSize( 250, 230 ).expandedTo( minimumSizeHint() ) );
}

WorkSheetSettings::~WorkSheetSettings()
{
}

void WorkSheetSettings::setRows( int rows )
{
  mRows->setValue( rows );
}

int WorkSheetSettings::rows() const
{
  return mRows->value();
}

void WorkSheetSettings::setColumns( int columns )
{
  mColumns->setValue( columns );
}

int WorkSheetSettings::columns() const
{
  return mColumns->value();
}

void WorkSheetSettings::setInterval( float interval )
{
  mInterval->setValue( interval );
}

float WorkSheetSettings::interval() const
{
  return mInterval->value();
}

void WorkSheetSettings::setSheetTitle( const QString &title )
{
  mSheetTitle->setText( title );
}

QString WorkSheetSettings::sheetTitle() const
{
  return mSheetTitle->text();
}


