/****************************************************************************
** Form implementation generated from reading ui file 'MultiMeterSettings.ui'
**
** Created: Tue Jul 18 02:10:57 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "MultiMeterSettings.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a MultiMeterSettings which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
MultiMeterSettings::MultiMeterSettings( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setProperty( "name", "MultiMeterSettings" );
    resize( 333, 251 ); 
    setProperty( "caption", i18n( "Form1"  ) );
    grid = new QGridLayout( this ); 
    grid->setSpacing( 6 );
    grid->setMargin( 11 );

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setProperty( "title", i18n( "Alarm for minimum value"  ) );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 0 );
    GroupBox1->layout()->setMargin( 0 );
    grid_2 = new QGridLayout( GroupBox1->layout() );
    grid_2->setAlignment( Qt::AlignTop );
    grid_2->setSpacing( 6 );
    grid_2->setMargin( 11 );

    lowerLimitActive = new QCheckBox( GroupBox1, "lowerLimitActive" );
    lowerLimitActive->setProperty( "text", i18n( "Enable Alarm"  ) );

    grid_2->addWidget( lowerLimitActive, 0, 0 );
    QSpacerItem* spacer = new QSpacerItem( 22, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    grid_2->addItem( spacer, 0, 1 );

    TextLabel2 = new QLabel( GroupBox1, "TextLabel2" );
    TextLabel2->setProperty( "text", i18n( "Lower Limit"  ) );

    grid_2->addWidget( TextLabel2, 0, 2 );

    lowerLimit = new QSpinBox( GroupBox1, "lowerLimit" );
    lowerLimit->setProperty( "maxValue", 99999999 );
    lowerLimit->setProperty( "enabled", QVariant( FALSE, 0 ) );

    grid_2->addWidget( lowerLimit, 0, 3 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 17, QSizePolicy::Fixed, QSizePolicy::Expanding );
    grid_2->addItem( spacer_2, 1, 1 );

    grid->addWidget( GroupBox1, 1, 0 );

    GroupBox1_2 = new QGroupBox( this, "GroupBox1_2" );
    GroupBox1_2->setProperty( "title", i18n( "Alarm for maximum value"  ) );
    GroupBox1_2->setColumnLayout(0, Qt::Vertical );
    GroupBox1_2->layout()->setSpacing( 0 );
    GroupBox1_2->layout()->setMargin( 0 );
    grid_3 = new QGridLayout( GroupBox1_2->layout() );
    grid_3->setAlignment( Qt::AlignTop );
    grid_3->setSpacing( 6 );
    grid_3->setMargin( 11 );

    upperLimitActive = new QCheckBox( GroupBox1_2, "upperLimitActive" );
    upperLimitActive->setProperty( "text", i18n( "Enable Alarm"  ) );

    grid_3->addWidget( upperLimitActive, 0, 0 );
    QSpacerItem* spacer_3 = new QSpacerItem( 23, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    grid_3->addItem( spacer_3, 0, 1 );

    TextLabel2_2 = new QLabel( GroupBox1_2, "TextLabel2_2" );
    TextLabel2_2->setProperty( "text", i18n( "Upper Limit"  ) );

    grid_3->addWidget( TextLabel2_2, 0, 2 );

    upperLimit = new QSpinBox( GroupBox1_2, "upperLimit" );
    upperLimit->setProperty( "maxValue", 99999999 );
    upperLimit->setProperty( "enabled", QVariant( FALSE, 0 ) );

    grid_3->addWidget( upperLimit, 0, 3 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 17, QSizePolicy::Fixed, QSizePolicy::Expanding );
    grid_3->addItem( spacer_4, 1, 1 );

    grid->addWidget( GroupBox1_2, 2, 0 );

    hbox = new QHBoxLayout; 
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );
    QSpacerItem* spacer_5 = new QSpacerItem( 88, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    hbox->addItem( spacer_5 );

    okButton = new QPushButton( this, "okButton" );
    okButton->setProperty( "text", i18n( "Ok"  ) );
    okButton->setProperty( "default", QVariant( TRUE, 0 ) );
    hbox->addWidget( okButton );

    applyButton = new QPushButton( this, "applyButton" );
    applyButton->setProperty( "text", i18n( "Apply"  ) );
    hbox->addWidget( applyButton );

    cancelButton = new QPushButton( this, "cancelButton" );
    cancelButton->setProperty( "text", i18n( "Cancel"  ) );
    hbox->addWidget( cancelButton );

    grid->addLayout( hbox, 3, 0 );

    hbox_2 = new QHBoxLayout; 
    hbox_2->setSpacing( 6 );
    hbox_2->setMargin( 0 );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setProperty( "text", i18n( "Title:"  ) );
    hbox_2->addWidget( TextLabel1 );

    title = new QLineEdit( this, "title" );
    hbox_2->addWidget( title );

    showUnit = new QCheckBox( this, "showUnit" );
    showUnit->setProperty( "text", i18n( "Show Unit"  ) );
    hbox_2->addWidget( showUnit );

    grid->addLayout( hbox_2, 0, 0 );

    // signals and slots connections
    connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( upperLimitActive, SIGNAL( toggled(bool) ), upperLimit, SLOT( setEnabled(bool) ) );
    connect( lowerLimitActive, SIGNAL( toggled(bool) ), lowerLimit, SLOT( setEnabled(bool) ) );

    // tab order
    setTabOrder( title, showUnit );
    setTabOrder( showUnit, lowerLimitActive );
    setTabOrder( lowerLimitActive, lowerLimit );
    setTabOrder( lowerLimit, upperLimitActive );
    setTabOrder( upperLimitActive, upperLimit );
    setTabOrder( upperLimit, okButton );
    setTabOrder( okButton, applyButton );
    setTabOrder( applyButton, cancelButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
MultiMeterSettings::~MultiMeterSettings()
{
    // no need to delete child widgets, Qt does it all for us
}

void MultiMeterSettings::applyClicked()
{
    qWarning( "MultiMeterSettings::applyClicked(): Not implemented yet!" );
}

