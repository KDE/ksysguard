/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>

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

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#include <QLineEdit>
#include <QSpinBox>

#include <kdebug.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kacceleratormanager.h>

#include "WorkSheet.h"
#include "WorkSheetSettings.h"

#include "Workspace.h"

Workspace::Workspace( QWidget* parent)
  : QTabWidget( parent )
{
  KAcceleratorManager::setNoAccel(this);
 
  connect( this, SIGNAL( currentChanged( int ) ),
           SLOT( updateCaption( int ) ) );

  this->setWhatsThis( i18n( "This is your work space. It holds your worksheets. You need "
                               "to create a new worksheet (Menu File->New) before "
                               "you can drag sensors here." ) );
}

Workspace::~Workspace()
{
  /* This workaround is necessary to prevent a crash when the last
   * page is not the current page. It seems like the the signal/slot
   * administration data is already deleted but slots are still
   * being triggered. TODO: I need to ask the Trolls about this. */

  disconnect( this, SIGNAL( currentChanged( int ) ), this,
              SLOT( updateCaption( int ) ) );
}

void Workspace::saveProperties( KConfig *cfg )
{
  cfg->writeEntry( "CurrentSheet", tabText(indexOf( currentWidget() )) );

  QStringList list;
  for(int i =0; i< mSheetList.size(); i++)
    if ( !mSheetList.at(i)->fileName().isEmpty() )
      list.append( mSheetList.at(i)->fileName() );

  cfg->writePathEntry( "SelectedSheets", list );
}

void Workspace::readProperties( KConfig *cfg )
{
  kDebug() << "Reading from " << cfg->group() << endl;
  QStringList selectedSheets = cfg->readEntry( "SelectedSheets", QStringList() );
  kDebug() << "Selected Sheets = " << selectedSheets << endl;
  //This is from KDE 3.5 - we should port it over 
//  QStringList custom_sheets_list = cfg->readPathListEntry( "Sheets" );
  
  if ( selectedSheets.isEmpty() ) {
   /* If SelectedSheets config entry is not there, then it's
    * probably the first time the user has started KSysGuard. We
    * then "restore" a special default configuration. */
    selectedSheets << "SystemLoad.sgrd";
    selectedSheets << "ProcessTable.sgrd";
  }
 
  KStandardDirs* kstd = KGlobal::dirs();
  QString filename;
  for ( QStringList::Iterator it = selectedSheets.begin(); it != selectedSheets.end(); ++it ) {
    filename = kstd->findResource( "data", "ksysguard/" + *it);
    if(!filename.isEmpty()) {
      restoreWorkSheet( filename);
    }
  }

  QString currentSheet = cfg->readEntry( "CurrentSheet" , "ProcessTable.sgrd");
  
  // Determine visible sheet.
  for(int i = 0; i < mSheetList.size(); i++) 
    if ( currentSheet == tabText(indexOf(mSheetList.at(i))) ) {
      setCurrentIndex(indexOf( mSheetList.at(i) ));
      break;
    }
}

QString Workspace::makeNameForNewSheet() const
{
  /* Find a name of the form "Sheet %d" that is not yet used by any
   * of the existing worksheets. */
  int i = 1;
  bool found = false;
  QString sheetName;
  KStandardDirs* kstd = KGlobal::dirs();
  do {
    sheetName = i18n( "Sheet %1" ,  i++ );
    //Check we don't have any existing files with this name
    found = !(kstd->findResource( "data", "ksysguard/" + sheetName + ".sgrd").isEmpty());
    
    //Check if we have any sheets with the same tab name or file name
    for(int i = 0; !found && i < mSheetList.size(); i++)
      if ( tabText(indexOf(mSheetList.at(i))) == sheetName  || sheetName+".sgrd" == mSheetList.at(i)->fileName())
        found = true;

  } while ( found );

  return sheetName;
}

void Workspace::newWorkSheet()
{
  /* Find a name of the form "Sheet %d" that is not yet used by any
   * of the existing worksheets. */
  QString sheetName  = makeNameForNewSheet();

  WorkSheetSettings dlg( this, false /*not locked.  New custom sheets aren't locked*/ );
  dlg.setSheetTitle( sheetName );
  if ( dlg.exec() ) {
    WorkSheet* sheet = new WorkSheet( dlg.rows(), dlg.columns(), dlg.interval(), 0 );
    sheet->setTitle( dlg.sheetTitle() );
    sheet->setFileName( sheetName + ".sgrd" );
    insertTab(-1, sheet, dlg.sheetTitle() );
    mSheetList.append( sheet );
    setCurrentIndex(indexOf( sheet ));
    connect( sheet, SIGNAL( titleChanged( QWidget* ) ),
	     SLOT( updateSheetTitle( QWidget* )));
  }
}

void Workspace::updateSheetTitle( QWidget* wdg )
{
  if ( wdg )
    setTabText( indexOf(wdg), static_cast<WorkSheet*>( wdg )->title() );
}

bool Workspace::saveOnQuit()
{
  kDebug() << "In saveOnQuit()" << endl;
  for(int i = 0; i < mSheetList.size(); i++) {
      if ( mSheetList.at(i)->fileName().isEmpty() ) {
        int res = KMessageBox::warningYesNoCancel( this,
                  i18n( "The worksheet '%1' contains unsaved data.\n"
                        "Do you want to save the worksheet?",
                    tabText(indexOf( mSheetList.at(i) )) ), QString(), KStdGuiItem::save(), KStdGuiItem::discard() );
        if ( res == KMessageBox::Yes )
          saveWorkSheet( mSheetList.at(i) );
        else if ( res == KMessageBox::Cancel )
          return false; // abort quit
      } else
        saveWorkSheet(mSheetList.at(i));
  }
  return true;
}

void Workspace::importWorkSheet()
{
  KFileDialog dlg( QString(), i18n( "*.sgrd|Sensor Files" ), this);

  KUrl url = dlg.getOpenURL( QString(), "*.sgrd", 0, i18n( "Select Worksheet to Import" ) );

  importWorkSheet( url );
}

void Workspace::importWorkSheet( const KUrl &url )
{
  if ( url.isEmpty() )
    return;

  /* It's probably not worth the effort to make this really network
   * transparent. Unless s/o beats me up I use this pseudo transparent
   * code. */
  QString tmpFile;
  KIO::NetAccess::download( url, tmpFile, this );

  // Import sheet from file.
  if ( !restoreWorkSheet( tmpFile ) )
    return;

  mSheetList.last()->setFileName( makeNameForNewSheet() + ".sgrd");
  
  KIO::NetAccess::removeTempFile( tmpFile );
}

bool Workspace::saveWorkSheet( WorkSheet *sheet )
{
  if ( !sheet ) {
    KMessageBox::sorry( this, i18n( "You do not have a worksheet that could be saved." ) );
    return false;
  }

  KStandardDirs* kstd = KGlobal::dirs();
  QString fileName = kstd->saveLocation( "data", "ksysguard") + sheet->fileName();

  if ( !sheet->save( fileName ) ) {
    return false;
  }
  return true;
}

void Workspace::exportWorkSheet()
{
  exportWorkSheet( (WorkSheet*)currentWidget() );
}

void Workspace::exportWorkSheet( WorkSheet *sheet )
{
  if ( !sheet ) {
    KMessageBox::sorry( this, i18n( "You do not have a worksheet that could be saved." ) );
    return;
  }

  QString fileName;
  do {
    KFileDialog dlg( QString(), "*.sgrd", this);
    fileName = dlg.getSaveFileName( tabText(indexOf( currentWidget() ))+ ".sgrd",
		                    "*.sgrd", this, i18n("Export Work Sheet") );
    if ( fileName.isEmpty() )
      return;

  } while ( !sheet->exportWorkSheet( fileName ) );

}

void Workspace::removeWorkSheet()
{
  WorkSheet *current = (WorkSheet*)currentWidget();

  if ( current ) {
    saveWorkSheet( current );

    removeTab(indexOf( current ));
    mSheetList.removeAll( current );
  } else {
    QString msg = i18n( "There are no worksheets that could be deleted." );
    KMessageBox::error( this, msg );
  }
}

void Workspace::removeAllWorkSheets()
{
  WorkSheet *sheet;
  while ( ( sheet = (WorkSheet*)currentWidget() ) != 0 ) {
    saveWorkSheet( sheet );
    removeTab(indexOf( sheet ));
    mSheetList.removeAll( sheet );
    delete sheet;
  }
}

void Workspace::removeWorkSheet( const QString &fileName )
{
  for(int i = 0; i < mSheetList.size(); i++) {
    WorkSheet *sheet = mSheetList.at(i);
    if ( sheet->fileName() == fileName ) {
      removeTab(indexOf( sheet ));
      mSheetList.removeAt( i );
      delete sheet;
      return;
    }
  }
}

bool Workspace::restoreWorkSheet( const QString &fileName)
{
  // extract filename without path
  QString baseName = fileName.right( fileName.length() - fileName.lastIndexOf( '/' ) - 1 );

  WorkSheet *sheet = new WorkSheet( 0 );
  sheet->setFileName( baseName );
  if ( !sheet->load( fileName ) ) {
    delete sheet;
    return false;
  }
  mSheetList.append( sheet );
  insertTab(-1, sheet, sheet->title() );
  setCurrentIndex(indexOf(sheet));
  
  return true;
}

void Workspace::cut()
{
  WorkSheet *current = (WorkSheet*)currentWidget();

  if ( current )
    current->cut();
}

void Workspace::copy()
{
  WorkSheet *current = (WorkSheet*)currentWidget();

  if ( current )
    current->copy();
}

void Workspace::paste()
{
  WorkSheet *current = (WorkSheet*)currentWidget();

  if ( current )
    current->paste();
}

void Workspace::configure()
{
  WorkSheet *current = (WorkSheet*)currentWidget();

  if ( !current )
    return;

  current->settings();
}

void Workspace::updateCaption( QWidget *wdg)
{
  updateCaption( indexOf(wdg) );
}
void Workspace::updateCaption( int index )
{
  WorkSheet *wdg = static_cast<WorkSheet *>(widget(index));
  if ( wdg )
    emit setCaption( tabText(index) );
  else
    emit setCaption( QString() );

  for( int i = 0; i < mSheetList.size(); i++)
    mSheetList.at(i)->setIsOnTop( mSheetList.at(i) == wdg );
}

void Workspace::applyStyle()
{
  if ( currentWidget() )
    ((WorkSheet*)currentWidget())->applyStyle();
}

#include "Workspace.moc"
