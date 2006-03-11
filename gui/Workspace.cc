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

#include <qlineedit.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

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

Workspace::Workspace( QWidget* parent, const char* name )
  : QTabWidget( parent, name )
{
  KAcceleratorManager::setNoAccel(this);
 
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
  cfg->writePathEntry( "WorkDir", mWorkDir );
  cfg->writeEntry( "CurrentSheet", tabLabel( currentWidget() ) );

  Q3PtrListIterator<WorkSheet> it( mSheetList);

  QStringList list;
  for ( int i = 0; it.current(); ++it, ++i )
    if ( !(*it)->fileName().isEmpty() )
      list.append( (*it)->fileName() );

  cfg->writePathEntry( "Sheets", list );
}

void Workspace::readProperties( KConfig *cfg )
{
  QString currentSheet;

  mWorkDir = cfg->readPathEntry( "WorkDir" );

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
    QStringList list = cfg->readPathListEntry( "Sheets" );
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
      restoreWorkSheet( *it );
  }

  // Determine visible sheet.
  Q3PtrListIterator<WorkSheet> it( mSheetList );
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
    Q3PtrListIterator<WorkSheet> it( mSheetList );
    found = false;
    for ( ; it.current() && !found; ++it )
      if ( tabLabel(*it) == sheetName )
        found = true;
  } while ( found );

  WorkSheetSettings dlg( this );
  dlg.setSheetTitle( sheetName );
  if ( dlg.exec() ) {
    WorkSheet* sheet = new WorkSheet( dlg.rows(), dlg.columns(), dlg.interval(), 0 );
    sheet->setTitle( dlg.sheetTitle() );
    insertTab( sheet, dlg.sheetTitle() );
    mSheetList.append( sheet );
    showPage( sheet );
    connect( sheet, SIGNAL( sheetModified( QWidget* ) ),
             SLOT( updateCaption( QWidget* ) ) );
    connect( sheet, SIGNAL( titleChanged( QWidget* ) ),
	     SLOT( updateSheetTitle( QWidget* )));
  }
}

void Workspace::updateSheetTitle( QWidget* wdg )
{
  if ( wdg )
    changeTab( wdg, static_cast<WorkSheet*>( wdg )->title() );
}

bool Workspace::saveOnQuit()
{
  Q3PtrListIterator<WorkSheet> it( mSheetList );
  for ( ; it.current(); ++it )
    if ( (*it)->modified() ) {
      if ( !mAutoSave || (*it)->fileName().isEmpty() ) {
        int res = KMessageBox::warningYesNoCancel( this,
                  i18n( "The worksheet '%1' contains unsaved data.\n"
                        "Do you want to save the worksheet?")
                  .arg( tabLabel( *it ) ), QString(), KStdGuiItem::save(), KStdGuiItem::discard() );
        if ( res == KMessageBox::Yes )
          saveWorkSheet( *it );
        else if ( res == KMessageBox::Cancel )
          return false; // abort quit
      } else
        saveWorkSheet(*it);
    }

  return true;
}

void Workspace::loadWorkSheet()
{
  KFileDialog dlg( 0, i18n( "*.sgrd|Sensor Files" ), this);
  dlg.setObjectName("LoadFileDialog");

  KUrl url = dlg.getOpenURL( mWorkDir, "*.sgrd", 0, i18n( "Select Worksheet to Load" ) );

  loadWorkSheet( url );
}

void Workspace::loadWorkSheet( const KUrl &url )
{
  if ( url.isEmpty() )
    return;

  /* It's probably not worth the effort to make this really network
   * transparent. Unless s/o beats me up I use this pseudo transparent
   * code. */
  QString tmpFile;
  KIO::NetAccess::download( url, tmpFile, this );
  mWorkDir = tmpFile.left( tmpFile.lastIndexOf( '/' ) );

  // Load sheet from file.
  if ( !restoreWorkSheet( tmpFile ) )
    return;

  /* If we have loaded a non-local file we clear the file name so that
   * the users is prompted for a new name for saving the file. */
  KUrl tmpFileUrl;
  tmpFileUrl.setPath( tmpFile );
  if ( tmpFileUrl != url.url() )
    mSheetList.last()->setFileName( QString() );
  KIO::NetAccess::removeTempFile( tmpFile );

  emit announceRecentURL( KUrl( url ) );
}

void Workspace::saveWorkSheet()
{
  saveWorkSheet( (WorkSheet*)currentWidget() );
}

void Workspace::saveWorkSheetAs()
{
  saveWorkSheetAs( (WorkSheet*)currentWidget() );
}

void Workspace::saveWorkSheet( WorkSheet *sheet )
{
  if ( !sheet ) {
    KMessageBox::sorry( this, i18n( "You do not have a worksheet that could be saved." ) );
    return;
  }

  QString fileName = sheet->fileName();
  if ( fileName.isEmpty() ) {
    KFileDialog dlg( 0, i18n( "*.sgrd|Sensor Files" ), this);
	dlg.setObjectName("LoadFileDialog");
    fileName = dlg.getSaveFileName( mWorkDir + "/" + tabLabel( sheet ) +
                                    ".sgrd", "*.sgrd", 0,
                                    i18n( "Save Current Worksheet As" ) );
    if ( fileName.isEmpty() )
      return;

    mWorkDir = fileName.left( fileName.lastIndexOf( '/' ) );

    // extract filename without path
    QString baseName = fileName.right( fileName.length() - fileName.lastIndexOf( '/' ) - 1 );

    // chop off extension (usually '.sgrd')
    baseName = baseName.left( baseName.lastIndexOf( '.' ) );
    changeTab( sheet, baseName );
  }

  /* If we cannot save the file is probably write protected. So we need
   * to ask the user for a new name. */
  if ( !sheet->save( fileName ) ) {
    saveWorkSheetAs( sheet );
    return;
  }

  /* Add file to recent documents menue. */
  KUrl url;
  url.setPath( fileName );
  emit announceRecentURL( url );
}

void Workspace::saveWorkSheetAs( WorkSheet *sheet )
{
  if ( !sheet ) {
    KMessageBox::sorry( this, i18n( "You do not have a worksheet that could be saved." ) );
    return;
  }

  QString fileName;
  do {
    KFileDialog dlg( 0, "*.sgrd", this);
	dlg.setObjectName("LoadFileDialog");
    fileName = dlg.getSaveFileName( mWorkDir + "/" + tabLabel( currentWidget() ) +
                                    ".sgrd", "*.sgrd" );
    if ( fileName.isEmpty() )
      return;

    mWorkDir = fileName.left( fileName.lastIndexOf( '/' ) );

    // extract filename without path
    QString baseName = fileName.right( fileName.length() - fileName.lastIndexOf( '/' ) - 1 );

    // chop off extension (usually '.sgrd')
    baseName = baseName.left( baseName.lastIndexOf( '.' ) );
    changeTab( sheet, baseName );
  } while ( !sheet->save( fileName ) );

  /* Add file to recent documents menue. */
  KUrl url;
  url.setPath( fileName );
  emit announceRecentURL( url );
}

void Workspace::deleteWorkSheet()
{
  WorkSheet *current = (WorkSheet*)currentWidget();

  if ( current ) {
    if ( current->modified() ) {
      if ( !mAutoSave || current->fileName().isEmpty() ) {
        int res = KMessageBox::warningYesNoCancel( this,
                            i18n( "The worksheet '%1' contains unsaved data.\n"
                                  "Do you want to save the worksheet?" )
                            .arg( tabLabel( current ) ), QString(), KStdGuiItem::save(), KStdGuiItem::discard() );
        if ( res == KMessageBox::Cancel )
          return;

        if ( res == KMessageBox::Yes )
          saveWorkSheet( current );
      } else
        saveWorkSheet( current );
    }

    removePage( current );
    mSheetList.remove( current );
  } else {
    QString msg = i18n( "There are no worksheets that could be deleted." );
    KMessageBox::error( this, msg );
  }
}

void Workspace::removeAllWorkSheets()
{
  WorkSheet *sheet;
  while ( ( sheet = (WorkSheet*)currentWidget() ) != 0 ) {
    removePage( sheet );
    mSheetList.remove( sheet );
  }
}

void Workspace::deleteWorkSheet( const QString &fileName )
{
  Q3PtrListIterator<WorkSheet> it( mSheetList );
  for ( ; it.current(); ++it )
    if ( (*it)->fileName() == fileName ) {
      removePage( *it );
      mSheetList.remove( *it );
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
  QString baseName = tmpStr.right( tmpStr.length() - tmpStr.lastIndexOf( '/' ) - 1 );

  // chop off extension (usually '.sgrd')
  baseName = baseName.left( baseName.lastIndexOf( '.' ) );

  WorkSheet *sheet = new WorkSheet( 0 );
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

void Workspace::updateCaption( QWidget* wdg )
{
  if ( wdg )
    emit setCaption( tabLabel( wdg ), ((WorkSheet*)wdg)->modified() );
  else
    emit setCaption( QString(), false );

  for ( WorkSheet* s = mSheetList.first(); s != 0; s = mSheetList.next() )
    ((WorkSheet*)s)->setIsOnTop( s == wdg );
}

void Workspace::applyStyle()
{
  if ( currentWidget() )
    ((WorkSheet*)currentWidget())->applyStyle();
}

void Workspace::showProcesses()
{
  KStandardDirs* kstd = KGlobal::dirs();
  kstd->addResourceType( "data", "share/apps/ksysguard" );

  QString file = kstd->findResource( "data", "ProcessTable.sgrd" );
  if ( !file.isEmpty() )
    restoreWorkSheet( file );
  else
    KMessageBox::error( this, i18n( "Cannot find file ProcessTable.sgrd." ) );
}

#include "Workspace.moc"
