/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

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

#include <QClipboard>
#include <QCursor>
#include <q3dragobject.h>
#include <QLayout>
//Added by qt3to4:
#include <QTextStream>
#include <QGridLayout>
#include <QEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QCustomEvent>
#include <QFile>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>

#include <SensorManager.h>

#include "DancingBars.h"
#include "DummyDisplay.h"
#include "FancyPlotter.h"
#include "ListView.h"
#include "LogFile.h"
#include "MultiMeter.h"
#include "ProcessController.h"
#include "SensorLogger.h"
#include "WorkSheet.h"
#include "WorkSheetSettings.h"
#include "SensorFrame.h"

WorkSheet::WorkSheet( QWidget *parent )
  : QWidget( parent )
{
  mGridLayout = 0;
  mRows = mColumns = 0;
  mDisplayList = 0;
  mModified = false;
  mFileName = "";

  setAcceptDrops( true );
}

WorkSheet::WorkSheet( uint rows, uint columns, uint interval, QWidget* parent )
  : QWidget( parent)
{
  mRows = mColumns = 0;
  mGridLayout = 0;
  mDisplayList = 0;
  updateInterval( interval );
  mModified = false;
  mFileName = "";

  createGrid( rows, columns );

  // Initialize worksheet with dummy displays.
  for ( uint r = 0; r < mRows; ++r )
    for ( uint c = 0; c < mColumns; ++c )
      replaceDisplay( r, c );

  mGridLayout->activate();

  setAcceptDrops( true );
}

WorkSheet::~WorkSheet()
{
}

bool WorkSheet::load( const QString &fileName )
{
  setModified( false );

  mFileName = fileName;
  QFile file( mFileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    KMessageBox::sorry( this, i18n( "Cannot open the file %1." ,  mFileName ) );
    return false;
  }

  QDomDocument doc;

  // Read in file and check for a valid XML header.
  if ( !doc.setContent( &file) ) {
    KMessageBox::sorry( this, i18n( "The file %1 does not contain valid XML." ,
                          mFileName ) );
    return false;
  }

  // Check for proper document type.
  if ( doc.doctype().name() != "KSysGuardWorkSheet" ) {
    KMessageBox::sorry( this, i18n( "The file %1 does not contain a valid worksheet "
                                    "definition, which must have a document type 'KSysGuardWorkSheet'.",
                          mFileName ) );
    return false;
  }

  // Check for proper size.
  QDomElement element = doc.documentElement();
  updateInterval( element.attribute( "interval" ).toUInt() );
  if ( updateInterval() < 1 || updateInterval() > 300 )
    updateInterval( 2 );

  bool rowsOk, columnsOk;
  uint rows = element.attribute( "rows" ).toUInt( &rowsOk );
  uint columns = element.attribute( "columns" ).toUInt( &columnsOk );
  if ( !( rowsOk && columnsOk ) ) {
    KMessageBox::sorry( this, i18n("The file %1 has an invalid worksheet size.",
                          mFileName ) );
    return false;
  }

  createGrid( rows, columns );

  int i;
  /* Load lists of hosts that are needed for the work sheet and try
   * to establish a connection. */
  QDomNodeList dnList = element.elementsByTagName( "host" );
  for ( i = 0; i < dnList.count(); ++i ) {
    QDomElement element = dnList.item( i ).toElement();
    bool ok;
    int port = element.attribute( "port" ).toInt( &ok );
    if ( !ok )
      port = -1;
    KSGRD::SensorMgr->engage( element.attribute( "name" ),
                              element.attribute( "shell" ),
                              element.attribute( "command" ), port );
  }

  // Load the displays and place them into the work sheet.
  dnList = element.elementsByTagName( "display" );
  for ( i = 0; i < dnList.count(); ++i ) {
    QDomElement element = dnList.item( i ).toElement();
    uint row = element.attribute( "row" ).toUInt();
    uint column = element.attribute( "column" ).toUInt();
    if ( row >= mRows || column >= mColumns) {
      kDebug(1215) << "Row or Column out of range (" << row << ", "
                    << column << ")" << endl;
      return false;
    }
    kDebug() << "loading " << fileName << endl;
    replaceDisplay( row, column, element );
  }

  // Fill empty cells with dummy displays
  for ( uint r = 0; r < mRows; ++r )
    for ( uint c = 0; c < mColumns; ++c )
      if ( !mDisplayList[ r ][ c ] )
        replaceDisplay( r, c );

  setModified( false );

  return true;
}

bool WorkSheet::save( const QString &fileName )
{
  kDebug() << "SAVING - IN WORKSHEET" << endl;
  mFileName = fileName;

  QDomDocument doc( "KSysGuardWorkSheet" );
  doc.appendChild( doc.createProcessingInstruction(
                   "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

  // save work sheet information
  QDomElement ws = doc.createElement( "WorkSheet" );
  doc.appendChild( ws );
  ws.setAttribute( "interval", updateInterval() );
  ws.setAttribute( "rows", mRows );
  ws.setAttribute( "columns", mColumns );

  QStringList hosts;
  collectHosts( hosts );

  // save host information (name, shell, etc.)
  QStringList::Iterator it;
  for ( it = hosts.begin(); it != hosts.end(); ++it ) {
    QString shell, command;
    int port;

    if ( KSGRD::SensorMgr->hostInfo( *it, shell, command, port ) ) {
      QDomElement host = doc.createElement( "host" );
      ws.appendChild( host );
      host.setAttribute( "name", *it );
      host.setAttribute( "shell", shell );
      host.setAttribute( "command", command );
      host.setAttribute( "port", port );
    }
  }

  for ( uint r = 0; r < mRows; ++r )
    for (uint c = 0; c < mColumns; ++c )
      if ( QByteArray("DummyDisplay") != mDisplayList[ r ][ c ]->metaObject()->className()) {
        KSGRD::SensorDisplay* display = (KSGRD::SensorDisplay*)mDisplayList[ r ][ c ];
        QDomElement element = doc.createElement( "display" );
        ws.appendChild( element );
        element.setAttribute( "row", r );
        element.setAttribute( "column", c );
        element.setAttribute( "class", display->metaObject()->className() );

        display->saveSettings( doc, element );

	kDebug() << "SAVE Sort column is " << element.attribute("sortColumn", "1").toUInt() << endl;
      }

  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly ) ) {
    KMessageBox::sorry( this, i18n( "Cannot save file %1" ,  mFileName ) );
    return false;
  }

  QTextStream s( &file );
  kDebug() << "Saving to " << fileName << endl;
  s.setCodec( "UTF-8" );
  s << doc;
  file.close();

  setModified( false );

  return true;
}

void WorkSheet::cut()
{
  if ( !currentDisplay() || currentDisplay()->metaObject()->className() == QByteArray("DummyDisplay" ) )
    return;

  QClipboard* clip = QApplication::clipboard();

  clip->setText( currentDisplayAsXML() );

  removeDisplay( currentDisplay() );
}

void WorkSheet::copy()
{
  if ( !currentDisplay() || currentDisplay()->metaObject()->className() == QByteArray( "DummyDisplay" ) )
    return;

  QClipboard* clip = QApplication::clipboard();

  clip->setText( currentDisplayAsXML() );
}

void WorkSheet::paste()
{
  uint row, column;
  if ( !currentDisplay( &row, &column ) )
    return;

  QClipboard* clip = QApplication::clipboard();

  QDomDocument doc;
  /* Get text from clipboard and check for a valid XML header and
   * proper document type. */
  if ( !doc.setContent( clip->text() ) || doc.doctype().name() != "KSysGuardDisplay" ) {
    KMessageBox::sorry( this, i18n("The clipboard does not contain a valid display "
                        "description." ) );
    return;
  }

  QDomElement element = doc.documentElement();
  replaceDisplay( row, column, element );
}

void WorkSheet::setFileName( const QString &fileName )
{
  mFileName = fileName;
  setModified( true );
}

const QString& WorkSheet::fileName() const
{
  return mFileName;
}

bool WorkSheet::modified() const
{
  return mModified;
}

void WorkSheet::setTitle( const QString &title )
{
  mTitle = title;
  emit titleChanged(this);
}

const QString &WorkSheet::title() {
  return mTitle;
}

KSGRD::SensorDisplay *WorkSheet::addDisplay( const QString &hostName,
                                             const QString &sensorName,
                                             const QString &sensorType,
                                             const QString& sensorDescr,
                                             uint row, uint column )
{
  if ( !KSGRD::SensorMgr->engageHost( hostName ) ) {
    QString msg = i18n( "It is impossible to connect to \'%1\'." ,  hostName );
    KMessageBox::error( this, msg );

    return 0;
  }

  /* If the by 'row' and 'column' specified display is a QGroupBox dummy
   * display we replace the widget. Otherwise we just try to add
   * the new sensor to an existing display. */
  if ( mDisplayList[ row ][ column ]->metaObject()->className() == QByteArray( "DummyDisplay" ) ) {
    KSGRD::SensorDisplay* newDisplay = 0;
    /* If the sensor type is supported by more than one display
     * type we popup a menu so the user can select what display is
     * wanted. */
    if ( sensorType == "integer" || sensorType == "float" ) {
      KMenu pm;
      pm.addTitle( i18n( "Select Display Type" ) );
      QAction *a1 = pm.addAction( i18n( "&Line graph" ) );
      QAction *a2 = pm.addAction( i18n( "&Digital display" ) );
      QAction *a3 = pm.addAction( i18n( "&Bar graph" ) );
      QAction *a4 = pm.addAction( i18n( "Log to a &file" ) );
      QAction *execed = pm.exec( QCursor::pos() );
      if (execed == a1)
	newDisplay = new FancyPlotter( this, sensorDescr, false );
      else if (execed == a2)
	newDisplay = new MultiMeter( this, sensorDescr, false );
      else if (execed == a3)
	 newDisplay = new DancingBars( this, sensorDescr, false );
      else if (execed == a4)
	newDisplay = new SensorLogger( this, sensorDescr, false );
      else
	 return 0;
    } else if ( sensorType == "listview" )
      newDisplay = new ListView( this, sensorDescr, false );
    else if ( sensorType == "logfile" )
      newDisplay = new LogFile( this, sensorDescr, false );
    else if ( sensorType == "sensorlogger" )
      newDisplay = new SensorLogger( this, sensorDescr, false );
    else if ( sensorType == "table" )
      newDisplay = new ProcessController( this, sensorDescr);
    else {
      kDebug(1215) << "Unkown sensor type: " <<  sensorType << endl;
      return 0;
    }

    replaceDisplay( row, column, newDisplay );
  }

  mDisplayList[ row ][ column ]->addSensor( hostName, sensorName, sensorType, sensorDescr );

  setModified( true );

  return ((KSGRD::SensorDisplay*)mDisplayList[ row ][ column ] );
}

void WorkSheet::settings()
{
  WorkSheetSettings dlg( this );

  /* The sheet name should be changed with the "Save as..." function,
   * so we don't have to display the display frame. */
  dlg.setSheetTitle( mTitle );
  dlg.setRows( mRows );
  dlg.setColumns( mColumns );
  dlg.setInterval( updateInterval() );

  if ( dlg.exec() ) {
    updateInterval( dlg.interval() );
    for (uint r = 0; r < mRows; ++r)
      for (uint c = 0; c < mColumns; ++c)
        if ( mDisplayList[ r ][ c ]->useGlobalUpdateInterval() )
          mDisplayList[ r ][ c ]->setUpdateInterval( updateInterval() );

    resizeGrid( dlg.rows(), dlg.columns() );
    setTitle(dlg.sheetTitle());

    setModified( true );
  }
}

void WorkSheet::showPopupMenu( KSGRD::SensorDisplay *display )
{
  display->configureSettings();
}

void WorkSheet::setModified( bool modified )
{
  if ( modified != mModified ) {
    mModified = modified;
    if ( !modified )
      for ( uint r = 0; r < mRows; ++r )
        for ( uint c = 0; c < mColumns; ++c )
          mDisplayList[ r ][ c ]->setModified( false );

    emit sheetModified( this );
  }
}

void WorkSheet::applyStyle()
{
  for ( uint r = 0; r < mRows; ++r )
    for (uint c = 0; c < mColumns; ++c )
      mDisplayList[ r ][ c ]->applyStyle();
}

void WorkSheet::dragEnterEvent( QDragEnterEvent *e )
{
  e->setAccepted( Q3TextDrag::canDecode( e ) );
}

void WorkSheet::dropEvent( QDropEvent *e )
{
  QString dragObject;

  if ( Q3TextDrag::decode( e, dragObject) ) {
    // The host name, sensor name and type are separated by a ' '.
    QStringList parts = dragObject.split( ' ');

    QString hostName = parts[ 0 ];
    QString sensorName = parts[ 1 ];
    QString sensorType = parts[ 2 ];
    QString sensorDescr = parts[ 3 ];

    if ( hostName.isEmpty() || sensorName.isEmpty() || sensorType.isEmpty() ) {
      return;
    }

    /* Find the sensor display that is supposed to get the drop
     * event and replace or add sensor. */
    for ( uint r = 0; r < mRows; ++r )
      for ( uint c = 0; c < mColumns; ++c )
        if ( mDisplayList[ r ][ c ]->geometry().contains( e->pos() ) ) {
          addDisplay( hostName, sensorName, sensorType, sensorDescr, r, c );
          return;
        }
  }
}

QSize WorkSheet::sizeHint() const
{
  return QSize( 200,150 );
}

void WorkSheet::customEvent( QCustomEvent *e )
{
  if ( e->type() == QEvent::User ) {
    // SensorDisplays send out this event if they want to be removed.
    if ( KMessageBox::warningContinueCancel( this, i18n( "Do you really want to delete the display?" ),
      i18n("Delete Display"), KStdGuiItem::del() )
         == KMessageBox::Continue ) {
      removeDisplay( (KSGRD::SensorDisplay*)e->data() );
    }
  }
}

bool WorkSheet::replaceDisplay( uint row, uint column, QDomElement& element )
{
  QString classType = element.attribute( "class" );
  KSGRD::SensorDisplay* newDisplay;


  if ( classType == "FancyPlotter" )
    newDisplay = new FancyPlotter( 0, i18n("Dummy"), false );
  else if ( classType == "MultiMeter" )
    newDisplay = new MultiMeter( 0, i18n("Dummy"), false );
  else if ( classType == "DancingBars" )
    newDisplay = new DancingBars( 0, i18n("Dummy"), false );
  else if ( classType == "ListView" )
    newDisplay = new ListView( 0, i18n("Dummy"), false );
  else if ( classType == "LogFile" )
    newDisplay = new LogFile( 0, i18n("Dummy"), false );
  else if ( classType == "SensorLogger" )
    newDisplay = new SensorLogger( 0, i18n("Dummy"), false );
  else if ( classType == "ProcessController" )
    newDisplay = new ProcessController( 0, i18n("Dummy"));
  else {
    kDebug(1215) << "Unknown class " <<  classType << endl;
    return false;
  }

  if ( newDisplay->useGlobalUpdateInterval() )
    newDisplay->setUpdateInterval( updateInterval() );

  // load display specific settings
  if ( !newDisplay->restoreSettings( element ) )
    return false;

  replaceDisplay( row, column, newDisplay );

  return true;
}


void WorkSheet::replaceDisplay( uint row, uint column, KSGRD::SensorDisplay* newDisplay )
{
  // remove the old display at this location
  delete mDisplayList[ row ][ column ];

  // insert new display
  if ( !newDisplay )
    mDisplayList[ row ][ column ] = new DummyDisplay( this, "DummyDisplay" );
  else {
    mDisplayList[ row ][ column ] = newDisplay;
    if ( mDisplayList[ row ][ column ]->useGlobalUpdateInterval() )
      mDisplayList[ row ][ column ]->setUpdateInterval( updateInterval() );
    connect( newDisplay, SIGNAL( showPopupMenu( KSGRD::SensorDisplay* ) ),
             SLOT( showPopupMenu( KSGRD::SensorDisplay* ) ) );
    connect( newDisplay, SIGNAL( modified( bool ) ),
             SLOT( setModified( bool ) ) );
  }

  SensorFrame *frame = new SensorFrame(mDisplayList[ row ][ column ]);

  mGridLayout->addWidget( frame, row, column );
  if ( isVisible() ) {
    mDisplayList[ row ][ column ]->show();
  }

  setMinimumSize(sizeHint());

  setModified( true );
}

void WorkSheet::removeDisplay( KSGRD::SensorDisplay *display )
{
  if ( !display )
    return;

  for ( uint r = 0; r < mRows; ++r )
    for ( uint c = 0; c < mColumns; ++c )
      if ( mDisplayList[ r ][ c ] == display ) {
        replaceDisplay( r, c );
        setModified( true );
        return;
      }
}

void WorkSheet::collectHosts( QStringList &list )
{
  for ( uint r = 0; r < mRows; ++r )
    for ( uint c = 0; c < mColumns; ++c )
      if ( mDisplayList[ r ][ c ]->metaObject()->className() != QByteArray( "DummyDisplay" ) )
        ((KSGRD::SensorDisplay*)mDisplayList[ r ][ c ])->hosts( list );
}

void WorkSheet::createGrid( uint rows, uint columns )
{
  mRows = rows;
  mColumns = columns;

  // create grid layout with specified dimensions
  mGridLayout = new QGridLayout( this );
  mGridLayout->setSpacing( 5 );

  mDisplayList = new KSGRD::SensorDisplay**[ mRows ];
  for ( uint r = 0; r < mRows; ++r ) {
    mDisplayList[ r ] = new KSGRD::SensorDisplay*[ mColumns ];
    for ( uint c = 0; c < mColumns; ++c )
      mDisplayList[ r ][ c ] = 0;
  }

  /* set stretch factors for rows and columns */
  for ( uint r = 0; r < mRows; ++r )
    mGridLayout->setRowStretch( r, 100 );
  for ( uint c = 0; c < mColumns; ++c )
    mGridLayout->setColumnStretch( c, 100 );
}

void WorkSheet::resizeGrid( uint newRows, uint newColumns )
{
  uint r, c;

  /* Create new array for display pointers */
  KSGRD::SensorDisplay*** newDisplayList = new KSGRD::SensorDisplay**[ newRows ];
  for ( r = 0; r < newRows; ++r ) {
    newDisplayList[ r ] = new KSGRD::SensorDisplay*[ newColumns ];
    for ( c = 0; c < newColumns; ++c ) {
      if ( c < mColumns && r < mRows )
        newDisplayList[ r ][ c ] = mDisplayList[ r ][ c ];
      else
        newDisplayList[ r ][ c ] = 0;
    }
  }

  /* remove obsolete displays */
  for ( r = 0; r < mRows; ++r ) {
    for ( c = 0; c < mColumns; ++c )
      if ( r >= newRows || c >= newColumns )
        delete mDisplayList[ r ][ c ];
    delete mDisplayList[ r ];
  }
  delete [] mDisplayList;

  /* now we make the new display the regular one */
  mDisplayList = newDisplayList;

  /* create new displays */
  for ( r = 0; r < newRows; ++r )
    for ( c = 0; c < newColumns; ++c )
      if ( r >= mRows || c >= mColumns )
        replaceDisplay( r, c );

  /* set stretch factors for new rows and columns (if any) */
  for ( r = mRows; r < newRows; ++r )
    mGridLayout->setRowStretch( r, 100 );
  for ( c = mColumns; c < newColumns; ++c )
    mGridLayout->setColumnStretch( c, 100 );

  /* Obviously Qt does not shrink the size of the QGridLayout
   * automatically.  So we simply force the rows and columns that
   * are no longer used to have a strech factor of 0 and hence be
   * invisible. */
  for ( r = newRows; r < mRows; ++r )
    mGridLayout->setRowStretch( r, 0 );
  for ( c = newColumns; c < mColumns; ++c )
    mGridLayout->setColumnStretch( c, 0 );

  mRows = newRows;
  mColumns = newColumns;

  fixTabOrder();

  mGridLayout->activate();
}

KSGRD::SensorDisplay *WorkSheet::currentDisplay( uint *row, uint *column )
{
  for ( uint r = 0 ; r < mRows; ++r )
    for ( uint c = 0 ; c < mColumns; ++c )
      if ( mDisplayList[ r ][ c ]->hasFocus() ) {
        if ( row )
          *row = r;
        if ( column )
          *column = c;
        return ( mDisplayList[ r ][ c ] );
      }

  return 0;
}

void WorkSheet::fixTabOrder()
{
  for ( uint r = 0; r < mRows; ++r )
    for ( uint c = 0; c < mColumns; ++c ) {
      if ( c + 1 < mColumns )
        setTabOrder( mDisplayList[ r ][ c ], mDisplayList[ r ][ c + 1 ] );
      else if ( r + 1 < mRows )
        setTabOrder( mDisplayList[ r ][ c ], mDisplayList[ r + 1 ][ 0 ] );
    }
}

QString WorkSheet::currentDisplayAsXML()
{
  KSGRD::SensorDisplay* display = currentDisplay();
  if ( !display )
    return QString();

  /* We create an XML description of the current display. */
  QDomDocument doc( "KSysGuardDisplay" );
  doc.appendChild( doc.createProcessingInstruction(
                   "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

  QDomElement element = doc.createElement( "display" );
  doc.appendChild( element );
  element.setAttribute( "class", display->metaObject()->className() );
  display->saveSettings( doc, element );

  return doc.toString();
}

void WorkSheet::setIsOnTop( bool /* onTop */ )
{
/*
  for ( uint r = 0; r < mRows; ++r )
    for ( uint c = 0; c < mColumns; ++c )
      mDisplayList[ r ][ c ]->setIsOnTop( onTop );
*/
}

#include "WorkSheet.moc"
