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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

    $Id$
*/

#include <qlineedit.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "WorkSheet.h"
#include "WorkSheetSettings.h"

#include "Workspace.h"

Workspace::Workspace( QWidget* parent, const char* name )
  : QTabWidget( parent, name )
{
  mSheetList.setAutoDelete( true );
  mAutoSave = true;

  connect( this, SIGNAL( currentChanged( QWidget* ) ),
           SLOT( updateCaption( QWidget* ) ) );

  QWhatsThis::add( this, i18n( "This is your work space. It holds your worksheets. You need "
                               "to create a new worksheet (Menu File->New) before "
                               "you can drag sensors here." ) );
}

Workspace::~Workspace()
{
  /* This workaround is necessary to prevent a crash when the last
   * page is not the current page. It seems like the the signal/slot
   * administration data is already deleted but slots are still
   * being triggered. TODO: I need to ask the Trolls about this. */

  disconnect( this, SIGNAL( currentChanged( QWidget* ) ), this,
              SLOT( updateCaption( QWidget* ) ) );
}

void Workspace::saveProperties( KConfig *cfg )
{
  cfg->writeEntry( "WorkDir", mWorkDir );
  cfg->writeEntry( "CurrentSheet", tabLabel( currentPage() ) );

  QPtrListIterator<WorkSheet> it( mSheetList);

  QStringList list;
  for ( int i = 0; it.current(); ++it, ++i )
    if ( !(*it)->fileName().isEmpty() )
      list.append( (*it)->fileName() );

  cfg->writeEntry( "Sheets", list );
}

void Workspace::readProperties( KConfig *cfg )
{
  QString currentSheet;

  mWorkDir = cfg->readEntry( "WorkDir" );

  if ( mWorkDir.isEmpty() ) {
    /* If workDir is not specified in the config file, it's
     * probably the first time the user has started KSysGuard. We
     * then "restore" a special default configuration. */
    KStandardDirs* kstd = KGlobal::dirs();
    kstd->addResourceType( "data", "share/apps/ksysguard" );

    mWorkDir = kstd->saveLocation( "data", "ksysguard" );

    QString origFile = kstd->findResource( "data", "SystemLoad.sgrd" );
    QString newFile = mWorkDir + "/" + i18n( "System Load" ) + ".sgrd";
    if ( !origFile.isEmpty() )
      restoreWorkSheet( origFile, newFile );

    origFile = kstd->findResource( "data", "ProcessTable.sgrd" );
    newFile = mWorkDir + "/" + i18n( "Process Table" ) + ".sgrd";
    if ( !origFile.isEmpty() )
      restoreWorkSheet( origFile, newFile );

    currentSheet = i18n( "System Load" );
  } else {
    currentSheet = cfg->readEntry( "CurrentSheet" );
    QStringList list = cfg->readListEntry( "Sheets" );
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
      restoreWorkSheet( *it );
  }

  // Determine visible sheet.
  QPtrListIterator<WorkSheet> it( mSheetList );
  for ( ; it.current(); ++it )
    if ( currentSheet == tabLabel(*it) ) {
      showPage( *it );
      break;
    }
}

void Workspace::newWorkSheet()
{
  /* Find a name of the form "Sheet %d" that is not yet used by any
   * of the existing worksheets. */
  QString sheetName;
  bool found;

  int i = 1;
  do {
    sheetName = i18n( "Sheet %1" ).arg( i++ );
    QPtrListIterator<WorkSheet> it( mSheetList );
    found = false;
    for ( ; it.current() && !found; ++it )
      if ( tabLabel(*it) == sheetName )
        found = true;
  } while ( found );

  WorkSheetSettings dlg( this );
  dlg.setSheetTitle( sheetName );
	if ( dlg.exec() ) {
    WorkSheet* sheet = new WorkSheet( dlg.rows(), dlg.columns(), dlg.interval(), this );
    sheet->setTitle( dlg.sheetTitle() );
    insertTab( sheet, dlg.sheetTitle() );
    mSheetList.append( sheet );
    showPage( sheet );
    connect( sheet, SIGNAL( sheetModified( QWidget* ) ),
             SLOT( updateCaption( QWidget* ) ) );
  }
}

bool Workspace::saveOnQuit()
{
  QPtrListIterator<WorkSheet> it( mSheetList );
  for ( ; it.current(); ++it )
    if ( (*it)->modified() ) {
      if ( !mAutoSave || (*it)->fileName().isEmpty() ) {
        int res = KMessageBox::warningYesNoCancel( this,
                  i18n( "The worksheet '%1' contains unsaved data.\n"
                        "Do you want to save the worksheet?")
                  .arg( tabLabel( *it ) ) );
        if ( res == KMessageBox::Yes )
          saveWorkSheet( *it );
        else if ( res == KMessageBox::Cancel )
          return false;	// abort quit
      } else
        saveWorkSheet(*it);
    }

  return true;
}

void Workspace::loadWorkSheet()
{
  KFileDialog dlg( 0, i18n( "*.sgrd|Sensor Files" ), this,
                   "LoadFileDialog", true );

	KURL url = dlg.getOpenURL( mWorkDir, "*.sgrd", 0, i18n( "Select Worksheet to Load" ) );

  loadWorkSheet( url );
}

void Workspace::loadWorkSheet( const KURL &url )
{
  if ( url.isEmpty() )
    return;

  /* It's probably not worth the effort to make this really network
   * transparent. Unless s/o beats me up I use this pseudo transparent
   * code. */
  QString tmpFile;
  KIO::NetAccess::download( url, tmpFile );
  mWorkDir = tmpFile.left( tmpFile.findRev( '/' ) );

  // Load sheet from file.
  if ( !restoreWorkSheet( tmpFile ) )
    return;

  /* If we have loaded a non-local file we clear the file name so that
   * the users is prompted for a new name for saving the file. */
  KURL tmpFileUrl;
  tmpFileUrl.setPath( tmpFile );
  if ( tmpFileUrl != url.url() )
    mSheetList.last()->setFileName( QString::null );
  KIO::NetAccess::removeTempFile( tmpFile );

  emit announceRecentURL( KURL( url ) );
}

void Workspace::saveWorkSheet()
{
  saveWorkSheet( (WorkSheet*)currentPage() );
}

void Workspace::saveWorkSheetAs()
{
  saveWorkSheetAs( (WorkSheet*)currentPage() );
}

void Workspace::saveWorkSheet( WorkSheet *sheet )
{
  if ( !sheet ) {
    KMessageBox::sorry(	this, i18n( "You don't have a worksheet that could be saved!" ) );
    return;
  }

  QString fileName = sheet->fileName();
  if ( fileName.isEmpty() ) {
    KFileDialog dlg( 0, i18n( "*.sgrd|Sensor Files" ), this,
                     "LoadFileDialog", true );
    fileName = dlg.getSaveFileName( mWorkDir + "/" + tabLabel( sheet ) +
                                    ".sgrd", "*.sgrd", 0,
                                    i18n( "Save Current Worksheet As" ) );
    if ( fileName.isEmpty() )
      return;

    mWorkDir = fileName.left( fileName.findRev( '/' ) );

    // extract filename without path
    QString baseName = fileName.right( fileName.length() - fileName.findRev( '/' ) - 1 );

		// chop off extension (usually '.sgrd')
    baseName = baseName.left( baseName.findRev( '.' ) );
    changeTab( sheet, baseName );
  }

  /* If we cannot save the file is probably write protected. So we need
   * to ask the user for a new name. */
  if ( !sheet->save( fileName ) ) {
    saveWorkSheetAs( sheet );
    return;
  }

  /* Add file to recent documents menue. */
  KURL url;
  url.setPath( fileName );
  emit announceRecentURL( url );
}

void Workspace::saveWorkSheetAs( WorkSheet *sheet )
{
  if ( !sheet ) {
    KMessageBox::sorry( this, i18n( "You don't have a worksheet that could be saved!" ) );
    return;
  }

  QString fileName;
  do {
    KFileDialog dlg( 0, "*.sgrd", this, "LoadFileDialog", true );
    fileName = dlg.getSaveFileName( mWorkDir + "/" + tabLabel( currentPage() ) +
                                    ".sgrd", "*.sgrd" );
    if ( fileName.isEmpty() )
      return;

    mWorkDir = fileName.left( fileName.findRev( '/' ) );

    // extract filename without path
    QString baseName = fileName.right( fileName.length() - fileName.findRev( '/' ) - 1 );

    // chop off extension (usually '.sgrd')
    baseName = baseName.left( baseName.findRev( '.' ) );
    changeTab( sheet, baseName );
  } while ( !sheet->save( fileName ) );

  /* Add file to recent documents menue. */
  KURL url;
  url.setPath( fileName );
  emit announceRecentURL( url );
}

void Workspace::deleteWorkSheet()
{
  WorkSheet *current = (WorkSheet*)currentPage();

  if ( current ) {
    if ( current->modified() ) {
      if ( !mAutoSave || current->fileName().isEmpty() ) {
        int res = KMessageBox::warningYesNoCancel( this,
                            i18n( "The worksheet '%1' contains unsaved data.\n"
                                  "Do you want to save the worksheet?" )
                            .arg( tabLabel( current ) ) );
        if ( res == KMessageBox::Yes )
          saveWorkSheet( current );
      }	else
        saveWorkSheet( current );
    }

    removePage( current );
    mSheetList.remove( current );

    setMinimumSize( sizeHint() );
	} else {
    QString msg = i18n( "There are no worksheets that could be deleted!" );
    KMessageBox::error( this, msg );
  }
}

void Workspace::removeAllWorkSheets()
{
  WorkSheet *sheet;
  while ( ( sheet = (WorkSheet*)currentPage() ) != 0 ) {
    removePage( sheet );
    mSheetList.remove( sheet );
  }
}

void Workspace::deleteWorkSheet( const QString &fileName )
{
  QPtrListIterator<WorkSheet> it( mSheetList );
  for ( ; it.current(); ++it )
    if ( (*it)->fileName() == fileName ) {
      removePage( *it );
      mSheetList.remove( *it );

      setMinimumSize( sizeHint() );
      return;
    }
}

bool Workspace::restoreWorkSheet( const QString &fileName, const QString &newName )
{
  /* We might want to save the worksheet under a different name later. This
   * name can be specified by newName. If newName is empty we use the
   * original name to save the work sheet. */
  QString tmpStr;
  if ( newName.isEmpty() )
    tmpStr = fileName;
  else
    tmpStr = newName;

  // extract filename without path
  QString baseName = tmpStr.right( tmpStr.length() - tmpStr.findRev( '/' ) - 1 );

	// chop off extension (usually '.sgrd')
  baseName = baseName.left( baseName.findRev( '.' ) );

  WorkSheet *sheet = new WorkSheet( this );
	sheet->setTitle( baseName );
  insertTab( sheet, baseName );
  showPage( sheet );

  if ( !sheet->load( fileName ) ) {
    delete sheet;
    return false;
  }
  
  mSheetList.append( sheet );
  connect( sheet, SIGNAL( sheetModified( QWidget* ) ),
           SLOT( updateCaption( QWidget* ) ) );

  /* Force the file name to be the new name. This also sets the modified
   * flag, so that the file will get saved on exit. */
  if ( !newName.isEmpty() )
    sheet->setFileName( newName );

  return true;
}

void Workspace::cut()
{
  WorkSheet *current = (WorkSheet*)currentPage();

  if ( current )
    current->cut();
}

void Workspace::copy()
{
  WorkSheet *current = (WorkSheet*)currentPage();

  if ( current )
    current->copy();
}

void Workspace::paste()
{
  WorkSheet *current = (WorkSheet*)currentPage();

  if ( current )
    current->paste();
}

void Workspace::configure()
{
  WorkSheet *current = (WorkSheet*)currentPage();

  if ( !current )
    return;

  current->settings();
}

void Workspace::updateCaption( QWidget* wdg )
{
  if ( wdg )
    emit setCaption( tabLabel( wdg ), ((WorkSheet*)wdg)->modified() );
  else
    emit setCaption( QString::null, false );

  for ( WorkSheet* s = mSheetList.first(); s != 0; s = mSheetList.next() )
    ((WorkSheet*)s)->setIsOnTop( s == wdg );
}

void Workspace::applyStyle()
{
  if ( currentPage() )
    ((WorkSheet*)currentPage())->applyStyle();
}

void Workspace::showProcesses()
{
  KStandardDirs* kstd = KGlobal::dirs();
  kstd->addResourceType( "data", "share/apps/ksysguard" );

  QString file = kstd->findResource( "data", "ProcessTable.sgrd" );
  if ( !file.isEmpty() )
    restoreWorkSheet( file );
  else
    KMessageBox::error( this, i18n( "Cannot find file ProcessTable.sgrd!" ) );
}

#include "Workspace.moc"
