#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './ListViewSettings.ui'
**
** Created: Sun Apr 22 12:23:59 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "ListViewSettings.h"

#include <kcolorbtn.h>
#include <klocale.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a ListViewSettings which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ListViewSettings::ListViewSettings( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "ListViewSettings" );
    resize( 328, 215 ); 
    setCaption( i18n( "Listview Settings" ) );
    ListViewSettingsLayout = new QGridLayout( this ); 
    ListViewSettingsLayout->setSpacing( 6 );
    ListViewSettingsLayout->setMargin( 11 );

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setTitle( QString::null );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 0 );
    GroupBox1->layout()->setMargin( 0 );
    GroupBox1Layout = new QGridLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );
    GroupBox1Layout->setSpacing( 6 );
    GroupBox1Layout->setMargin( 11 );

    gridColorLabel = new QLabel( GroupBox1, "gridColorLabel" );
    gridColorLabel->setText( i18n( "GridColor" ) );
    QWhatsThis::add(  gridColorLabel, i18n( "Color for the ListView-Grid" ) );

    GroupBox1Layout->addWidget( gridColorLabel, 0, 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox1Layout->addItem( spacer, 0, 1 );

    gridColorButton = new KColorButton( GroupBox1, "gridColorButton" );

    GroupBox1Layout->addWidget( gridColorButton, 0, 2 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    GroupBox1Layout->addItem( spacer_2, 1, 1 );

    ListViewSettingsLayout->addWidget( GroupBox1, 1, 0 );

    GroupBox2 = new QGroupBox( this, "GroupBox2" );
    GroupBox2->setTitle( QString::null );
    GroupBox2->setColumnLayout(0, Qt::Vertical );
    GroupBox2->layout()->setSpacing( 0 );
    GroupBox2->layout()->setMargin( 0 );
    GroupBox2Layout = new QGridLayout( GroupBox2->layout() );
    GroupBox2Layout->setAlignment( Qt::AlignTop );
    GroupBox2Layout->setSpacing( 6 );
    GroupBox2Layout->setMargin( 11 );

    textColorLabel = new QLabel( GroupBox2, "textColorLabel" );
    textColorLabel->setText( i18n( "TextColor" ) );
    QWhatsThis::add(  textColorLabel, i18n( "Color for the ListView-Text" ) );

    GroupBox2Layout->addWidget( textColorLabel, 0, 0 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox2Layout->addItem( spacer_3, 0, 1 );

    textColorButton = new KColorButton( GroupBox2, "textColorButton" );

    GroupBox2Layout->addWidget( textColorButton, 0, 2 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    GroupBox2Layout->addItem( spacer_4, 1, 1 );

    ListViewSettingsLayout->addWidget( GroupBox2, 2, 0 );

    Layout3 = new QHBoxLayout; 
    Layout3->setSpacing( 6 );
    Layout3->setMargin( 0 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout3->addItem( spacer_5 );

    okButton = new QPushButton( this, "okButton" );
    okButton->setText( i18n( "OK" ) );
    okButton->setDefault( TRUE );
    Layout3->addWidget( okButton );

    applyButton = new QPushButton( this, "applyButton" );
    applyButton->setText( i18n( "Apply" ) );
    Layout3->addWidget( applyButton );

    cancelButton = new QPushButton( this, "cancelButton" );
    cancelButton->setText( i18n( "Cancel" ) );
    Layout3->addWidget( cancelButton );

    ListViewSettingsLayout->addLayout( Layout3, 4, 0 );

    // signals and slots connections
    connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );

    // tab order
    setTabOrder( gridColorButton, textColorButton );
    setTabOrder( textColorButton, okButton );
    setTabOrder( okButton, applyButton );
    setTabOrder( applyButton, cancelButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ListViewSettings::~ListViewSettings()
{
    // no need to delete child widgets, Qt does it all for us
}

void ListViewSettings::applyClicked()
{
    qWarning( "ListViewSettings::applyClicked(): Not implemented yet!" );
}

#include "ListViewSettings.moc"
