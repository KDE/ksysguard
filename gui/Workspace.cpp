/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>
    Copyright (c) 2006 John Tapsell <tapsell@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License version 2 or at your option version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <QLineEdit>
#include <QSpinBox>

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kacceleratormanager.h>
#include <kactioncollection.h>
#include <kmenu.h>
#include <knewstuff3/downloaddialog.h>

#include "WorkSheet.h"
#include "WorkSheetSettings.h"

#include "Workspace.h"
#include "ksysguard.h"

Workspace::Workspace( QWidget* parent)
  : KTabWidget( parent )
{
  KAcceleratorManager::setNoAccel(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  setDocumentMode(true);
  connect(&mDirWatch, SIGNAL(deleted(QString)), this, SLOT(removeWorkSheet(QString)));
}

Workspace::~Workspace()
{
}

void Workspace::saveProperties( KConfigGroup& cfg )
{
  QStringList list;
  for(int i =0; i< mSheetList.size(); i++)
    if ( !mSheetList.at(i)->fileName().isEmpty() )
      list.append( mSheetList.at(i)->fileName() );

  cfg.writePathEntry( "SelectedSheets", list );
  cfg.writeEntry( "currentSheet", currentIndex() );
}

void Workspace::readProperties( const KConfigGroup& cfg )
{
  QStringList selectedSheets = cfg.readPathEntry( "SelectedSheets", QStringList() );

  if ( selectedSheets.isEmpty() ) {
   /* If SelectedSheets config entry is not there, then it's
    * probably the first time the user has started KSysGuard. We
    * then "restore" a special default configuration. */
    selectedSheets << "ProcessTable.sgrd";
    selectedSheets << "SystemLoad2.sgrd";
  } else if(selectedSheets[0] != "ProcessTable.sgrd") {
    //We need to make sure that this is really is the process table on the first tab. No GUI way of changing this, but should make sure anyway.
    //Plus this migrates users from the kde3 setup
    selectedSheets.removeAll("ProcessTable.sgrd");
    selectedSheets.prepend( "ProcessTable.sgrd");
  }

  int oldSystemLoad = selectedSheets.indexOf("SystemLoad.sgrd");
  if(oldSystemLoad != -1) {
    selectedSheets.replace(oldSystemLoad, "SystemLoad2.sgrd");
  }

  KStandardDirs* kstd = KGlobal::dirs();
  QString filename;
  for ( QStringList::Iterator it = selectedSheets.begin(); it != selectedSheets.end(); ++it ) {
    filename = kstd->findResource( "data", "ksysguard/" + *it);
    if(!filename.isEmpty()) {
      restoreWorkSheet( filename, false);
    }
  }

  int idx = cfg.readEntry( "currentSheet", 0 );
  if (idx < 0 || idx > count() - 1) {
    idx = 0;
  }
  setCurrentIndex(idx);
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
      if ( tabText(indexOf(mSheetList.at(i))) == sheetName  || QString(sheetName+".sgrd") == mSheetList.at(i)->fileName())
        found = true;

  } while ( found );

  return sheetName;
}

void Workspace::refreshActiveWorksheet()
{
    WorkSheet* currentSheet = mSheetList.at(currentIndex());
    currentSheet->refreshSheet();
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
    connect( sheet, SIGNAL(titleChanged(QWidget*)),
	     SLOT(updateSheetTitle(QWidget*)));
  }
}
void Workspace::contextMenu (int index, const QPoint &point) {
//  KMenu pm;
  Q_UNUSED(index);
  Q_UNUSED(point);
//  QAction *new_worksheet = pm.addAction( Toplevel->actionCollection()->action("new_worksheet") );

 // QAction *action = pm.exec( point );


}
void Workspace::updateSheetTitle( QWidget* wdg )
{
  if ( wdg )
    setTabText( indexOf(wdg), static_cast<WorkSheet*>( wdg )->translatedTitle() );
}

bool Workspace::saveOnQuit()
{
  for(int i = 0; i < mSheetList.size(); i++) {
      if ( mSheetList.at(i)->fileName().isEmpty() ) {
        int res = KMessageBox::warningYesNoCancel( this,
                  i18n( "The tab '%1' contains unsaved data.\n"
                        "Do you want to save the tab?",
                    tabText(indexOf( mSheetList.at(i) )) ), QString(), KStandardGuiItem::save(), KStandardGuiItem::discard() );
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
  KUrl url = KFileDialog::getOpenUrl( QString(), i18n("*.sgrd|Sensor Files (*.sgrd)"), this, i18n( "Select Tab File to Import" ) );

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
    KMessageBox::sorry( this, i18n( "You do not have a tab that could be saved." ) );
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
    KMessageBox::sorry( this, i18n( "You do not have a tab that could be saved." ) );
    return;
  }

  QString fileName;
  do {
    fileName = KFileDialog::getSaveFileName( QString(tabText(indexOf( currentWidget() ))+ ".sgrd"),
		                    QLatin1String("*.sgrd"), this, i18n("Export Tab") );
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
    QString msg = i18n( "There are no tabs that could be deleted." );
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
  QString baseName = fileName.right( fileName.length() - fileName.lastIndexOf( '/' ) - 1 );
  for(int i = 0; i < mSheetList.size(); i++) {
    WorkSheet *sheet = mSheetList.at(i);
    if ( sheet->fileName() == baseName ) {
      removeTab(indexOf( sheet ));
      mSheetList.removeAt( i );
      delete sheet;
      return;
    }
  }
}

WorkSheet *Workspace::currentWorkSheet()
{
    return (WorkSheet*)currentWidget();
}
void Workspace::uploadHotNewWorksheet()
{
    WorkSheet *currentWorksheet = currentWorkSheet();
    if(!currentWorksheet)
        return;

    KMessageBox::information(this, i18n("<qt>To propose the current custom tab as a new System Monitor tab, email <br><a href=\"file:%1\">%2</a><br> to <a href=\"mailto:john.tapsell@kde.org?subject='System Monitor Tab'&attach='file://%2'\">john.tapsell@kde.org</a></qt>", currentWorksheet->fullFileName().section('/',0,-2), currentWorksheet->fullFileName()), i18n("Upload custom System Monitor tab"), QString::null, KMessageBox::AllowLink);
}
void Workspace::getHotNewWorksheet()
{
    KNS3::DownloadDialog dialog("ksysguard.knsrc");
    if( dialog.exec() == QDialog::Rejected )
        return;

    KNS3::Entry::List entries = dialog.installedEntries();
    foreach(KNS3::Entry entry, entries) {
        if(!entry.installedFiles().isEmpty()) {
            QString filename = entry.installedFiles().first();
            restoreWorkSheet(filename, true);
        }
    }
}

bool Workspace::restoreWorkSheet( const QString &fileName, bool switchToTab)
{
  // extract filename without path
  QString baseName = fileName.right( fileName.length() - fileName.lastIndexOf( '/' ) - 1 );

  foreach( WorkSheet *sheet, mSheetList ) {
	  if(sheet->fileName() == baseName)
		  return false; //Don't add the same sheet twice
  }

  WorkSheet *sheet = new WorkSheet( 0 );
  sheet->setFileName( baseName );
  if ( !sheet->load( fileName ) ) {
    delete sheet;
    return false;
  }
  mSheetList.append( sheet );

  connect( sheet, SIGNAL(titleChanged(QWidget*)),
    SLOT(updateSheetTitle(QWidget*)));

  insertTab(-1, sheet, sheet->translatedTitle() );
  if(switchToTab)
   setCurrentIndex(indexOf(sheet));

  //Watch the file incase it is deleted
  mDirWatch.addFile(fileName);

  return true;
}

void Workspace::cut()
{
  WorkSheet *current = currentWorkSheet();

  if ( current )
    current->cut();
}

void Workspace::copy()
{
  WorkSheet *current = currentWorkSheet();

  if ( current )
    current->copy();
}

void Workspace::paste()
{
  WorkSheet *current = currentWorkSheet();

  if ( current )
    current->paste();
}

void Workspace::configure()
{
  WorkSheet *current = currentWorkSheet();

  if ( current )
      current->settings();
}

void Workspace::applyStyle()
{
  WorkSheet *current = currentWorkSheet();
  if ( current )
    current->applyStyle();
}

#include "Workspace.moc"
