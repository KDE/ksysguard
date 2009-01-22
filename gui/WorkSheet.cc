/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

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

#include <QClipboard>
#include <QCursor>
#include <QLayout>
#include <QTextStream>
#include <QGridLayout>
#include <QEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QFile>
#include <QByteArray>
#include <QApplication>

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

WorkSheet::WorkSheet( QWidget *parent )
  : QWidget( parent )
{
  mGridLayout = 0;
  mRows = mColumns = 0;
  mDisplayList = 0;
  mLocalProcessController = NULL;
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  setAcceptDrops( true );
}

WorkSheet::WorkSheet( uint rows, uint columns, float interval, QWidget* parent )
  : QWidget( parent)
{
  mRows = mColumns = 0;
  mGridLayout = 0;
  mDisplayList = 0;
  setUpdateInterval( interval );

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
  for ( int r = 0; r < mRows; ++r ) {
    for ( int c = 0; c < mColumns; ++c )
      delete mDisplayList[ r ][ c ];
    delete mDisplayList[ r ];
  }
  delete [] mDisplayList;
}

bool WorkSheet::load( const QString &fileName )
{
  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    KMessageBox::sorry( this, i18n( "Cannot open the file %1." ,  fileName ) );
    return false;
  }

  QDomDocument doc;

  // Read in file and check for a valid XML header.
  if ( !doc.setContent( &file) ) {
    KMessageBox::sorry( this, i18n( "The file %1 does not contain valid XML." ,
                          fileName ) );
    return false;
  }

  // Check for proper document type.
  if ( doc.doctype().name() != "KSysGuardWorkSheet" ) {
    KMessageBox::sorry( this, i18n( "The file %1 does not contain a valid worksheet "
                                    "definition, which must have a document type 'KSysGuardWorkSheet'.",
                          fileName ) );
    return false;
  }

  // Check for proper size.
  QDomElement element = doc.documentElement();
  float interval = element.attribute( "interval", "0.5" ).toFloat();
  if( interval  <0.01 || interval > 100000 )  //make sure the interval is fairly sane
    interval = 0.5;

  setUpdateInterval(interval);

  mTitle = element.attribute( "title");
  mTranslatedTitle = mTitle.isEmpty() ? "" : i18n(mTitle.toUtf8());
  bool ok;
  mSharedSettings.locked = element.attribute( "locked" ).toUInt( &ok );
  if(!ok) mSharedSettings.locked = false;
  
  bool rowsOk, columnsOk;
  uint rows = element.attribute( "rows" ).toUInt( &rowsOk );
  uint columns = element.attribute( "columns" ).toUInt( &columnsOk );
  if ( !( rowsOk && columnsOk ) ) {
    KMessageBox::sorry( this, i18n("The file %1 has an invalid worksheet size.",
                          fileName ) );
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
  //if no hosts are specified, at least connect to localhost
  if(dnList.count() == 0)
    KSGRD::SensorMgr->engage( "localhost", "", "ksysguardd", -1);

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
    replaceDisplay( row, column, element );
  }

  // Fill empty cells with dummy displays
  for ( uint r = 0; r < mRows; ++r )
    for ( uint c = 0; c < mColumns; ++c )
      if ( !mDisplayList[ r ][ c ] )
        replaceDisplay( r, c );
  mFullFileName = fileName;
  return true;
}

bool WorkSheet::save( const QString &fileName )
{
  return exportWorkSheet(fileName);
}

bool WorkSheet::exportWorkSheet( const QString &fileName )
{
  QDomDocument doc( "KSysGuardWorkSheet" );
  doc.appendChild( doc.createProcessingInstruction(
                   "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

  // save work sheet information
  QDomElement ws = doc.createElement( "WorkSheet" );
  doc.appendChild( ws );
  ws.setAttribute( "title", mTitle );
  ws.setAttribute( "locked", mSharedSettings.locked?"1":"0" );
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
      }

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly ) ) {
    KMessageBox::sorry( this, i18n( "Cannot save file %1" ,  fileName ) );
    return false;
  }

  QTextStream s( &file );
  s.setCodec( "UTF-8" );
  s << doc;
  file.close();

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
}

QString WorkSheet::fullFileName() const
{
  return mFullFileName;
}

QString WorkSheet::fileName() const
{
  return mFileName;
}

void WorkSheet::setTitle( const QString &title )
{
  mTitle = title;
  mTranslatedTitle = mTitle.isEmpty() ? "" : i18n(mTitle.toUtf8());
  emit titleChanged(this);
}

QString WorkSheet::translatedTitle() const {
  return mTranslatedTitle;
}

QString WorkSheet::title() const {
  return mTitle;
}

KSGRD::SensorDisplay *WorkSheet::addDisplay( const QString &hostName,
                                             const QString &sensorName,
                                             const QString &sensorType,
                                             const QString& sensorDescr,
                                             uint row, uint column )
{

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
      if (execed == a1) {
        newDisplay = new FancyPlotter( this, sensorDescr, &mSharedSettings );
      }
      else if (execed == a2) {
        newDisplay = new MultiMeter( this, sensorDescr, &mSharedSettings);
      }
      else if (execed == a3) {
        newDisplay = new DancingBars( this, sensorDescr, &mSharedSettings);
      }
      else if (execed == a4) {
        newDisplay = new SensorLogger( this, sensorDescr, &mSharedSettings);
      }
      else {
        return 0;
      }
    } else if ( sensorType == "listview" ) {
      newDisplay = new ListView( this, sensorDescr, &mSharedSettings);
    }
    else if ( sensorType == "logfile" ) {
      newDisplay = new LogFile( this, sensorDescr, &mSharedSettings );
    }
    else if ( sensorType == "sensorlogger" ) {
      newDisplay = new SensorLogger( this, sensorDescr, &mSharedSettings );
    }
    else if ( sensorType == "table" ) {
      if(!mLocalProcessController && (hostName.isEmpty() || hostName == "localhost")) {
        mLocalProcessController = new ProcessController( this, sensorDescr, &mSharedSettings);
	newDisplay = mLocalProcessController;
      } else 
        newDisplay = new ProcessController( this, sensorDescr, &mSharedSettings);
    }
    else {
      kDebug(1215) << "Unknown sensor type: " <<  sensorType;
      return 0;
    }

    newDisplay->applyStyle();

    connect(&mTimer, SIGNAL( timeout()), newDisplay, SLOT( timerTick()));
    replaceDisplay( row, column, newDisplay );
  }

  mDisplayList[ row ][ column ]->addSensor( hostName, sensorName, sensorType, sensorDescr );

  return ((KSGRD::SensorDisplay*)mDisplayList[ row ][ column ] );
}

void WorkSheet::settings()
{
  WorkSheetSettings dlg( this, mSharedSettings.locked );
  dlg.setSheetTitle( mTranslatedTitle );
  dlg.setInterval( updateInterval() );

  if(!mSharedSettings.locked) {
    dlg.setRows( mRows );
    dlg.setColumns( mColumns );
  }

  if ( dlg.exec() ) {
    setUpdateInterval( dlg.interval() );

    if (!mSharedSettings.locked)
      resizeGrid( dlg.rows(), dlg.columns() );

    if(mTranslatedTitle != dlg.sheetTitle()) { //Title has changed
      if(mRows == 1 && mColumns ==1) {
        mDisplayList[ 0 ][ 0 ]->setTitle(dlg.sheetTitle());
      } else {
        setTitle(dlg.sheetTitle());
      }
    }
  }
}

void WorkSheet::showPopupMenu( KSGRD::SensorDisplay *display )
{
  display->configureSettings();
}

void WorkSheet::applyStyle()
{
  for ( uint r = 0; r < mRows; ++r )
    for (uint c = 0; c < mColumns; ++c )
      mDisplayList[ r ][ c ]->applyStyle();
}

void WorkSheet::dragEnterEvent( QDragEnterEvent* event) 
{
  if ( !event->mimeData()->hasFormat("application/x-ksysguard") )
    return;
  event->accept();
}
void WorkSheet::dragMoveEvent( QDragMoveEvent *event )
{

  /* Find the sensor display that is supposed to get the drop
   * event and replace or add sensor. */
  const QPoint globalPos = mapToGlobal( event->pos() );
  for ( uint r = 0; r < mRows; ++r ) {
    for ( uint c = 0; c < mColumns; ++c ) {
      const QSize displaySize = mDisplayList[ r ][ c ]->size();

      const QPoint displayPoint( displaySize.width(), displaySize.height() );

      const QRect widgetRect = QRect( mDisplayList[ r ][ c ]->mapToGlobal( QPoint( 0, 0 ) ),
                                      mDisplayList[ r ][ c ]->mapToGlobal( displayPoint ) );


      if ( widgetRect.contains( globalPos ) ) {
        QByteArray widgetType = mDisplayList[ r ][ c ]->metaObject()->className();
        if(widgetType == "MultiMeter" || widgetType == "ProcessController" || widgetType == "table")
		event->ignore(widgetRect);
	else if(widgetType != "Dumm")
  		event->accept(widgetRect);
        return;
      }
    }
  }
}

void WorkSheet::dropEvent( QDropEvent *event )
{
  if ( !event->mimeData()->hasFormat("application/x-ksysguard") )
    return;

  const QString dragObject = QString::fromUtf8(event->mimeData()->data("application/x-ksysguard"));

  // The host name, sensor name and type are separated by a ' '.
  QStringList parts = dragObject.split( ' ');

  QString hostName = parts[ 0 ];
  QString sensorName = parts[ 1 ];
  QString sensorType = parts[ 2 ];
  QString sensorDescr = parts[ 3 ];

  if ( hostName.isEmpty() || sensorName.isEmpty() || sensorType.isEmpty() )
    return;

  /* Find the sensor display that is supposed to get the drop
   * event and replace or add sensor. */
  const QPoint globalPos = mapToGlobal( event->pos() );
  for ( uint r = 0; r < mRows; ++r ) {
    for ( uint c = 0; c < mColumns; ++c ) {
      const QSize displaySize = mDisplayList[ r ][ c ]->size();

      const QPoint displayPoint( displaySize.width(), displaySize.height() );

      const QRect widgetRect = QRect( mDisplayList[ r ][ c ]->mapToGlobal( QPoint( 0, 0 ) ),
                                      mDisplayList[ r ][ c ]->mapToGlobal( displayPoint ) );


      if ( widgetRect.contains( globalPos ) ) {
        addDisplay( hostName, sensorName, sensorType, sensorDescr, r, c );
        return;
      }
    }
  }
}

QSize WorkSheet::sizeHint() const
{
  return QSize( 800,600 );
}

bool WorkSheet::event( QEvent *e )
{
  if ( e->type() == QEvent::User ) {
    // SensorDisplays send out this event if they want to be removed.
    if ( KMessageBox::warningContinueCancel( this, i18n( "Do you really want to delete the display?" ),
      i18n("Delete Display"), KStandardGuiItem::del() )
         == KMessageBox::Continue ) {
      KSGRD::SensorDisplay::DeleteEvent *event = static_cast<KSGRD::SensorDisplay::DeleteEvent*>( e );
      removeDisplay( event->display() );

      return true;
    }
  }

  return QWidget::event( e );
}

bool WorkSheet::replaceDisplay( uint row, uint column, QDomElement& element )
{
  QString classType = element.attribute( "class" );
  KSGRD::SensorDisplay* newDisplay = 0;


  if ( classType == "FancyPlotter" ) {
    newDisplay = new FancyPlotter( 0, i18n("Dummy"), &mSharedSettings );
  }
  else if ( classType == "MultiMeter" )
    newDisplay = new MultiMeter( 0, i18n("Dummy"), &mSharedSettings );
  else if ( classType == "DancingBars" )
    newDisplay = new DancingBars( 0, i18n("Dummy"), &mSharedSettings );
  else if ( classType == "ListView" )
    newDisplay = new ListView( 0, i18n("Dummy"), &mSharedSettings );
  else if ( classType == "LogFile" )
    newDisplay = new LogFile( 0, i18n("Dummy"), &mSharedSettings );
  else if ( classType == "SensorLogger" )
    newDisplay = new SensorLogger( 0, i18n("Dummy"), &mSharedSettings );
  else if ( classType == "ProcessController" ) {
    if(!mLocalProcessController) {
      kDebug() << "Found process controller"; 
      mLocalProcessController = new ProcessController( 0, i18n("Dummy"), &mSharedSettings);
      newDisplay = mLocalProcessController;
    }
  } else {
    kDebug(1215) << "Unknown class " <<  classType;
    return false;
  }

  connect(&mTimer, SIGNAL( timeout()), newDisplay, SLOT( timerTick()));

  // load display specific settings
  if ( !newDisplay->restoreSettings( element ) )
    return false;

  replaceDisplay( row, column, newDisplay );

  return true;
}


void WorkSheet::replaceDisplay( uint row, uint column, KSGRD::SensorDisplay* newDisplay )
{
  // remove the old display && sensor frame at this location
  if ( mDisplayList[ row ][ column ] ) {
    if(mDisplayList[ row ][ column ] == mLocalProcessController) 
      mLocalProcessController = NULL;
    if ( qstrcmp( mDisplayList[ row ][ column ]->parent()->metaObject()->className(), "SensorFrame" ) == 0 ) {
      delete mDisplayList[ row ][ column ]->parent(); // destroys the child (display) as well
    } else {
      delete mDisplayList[ row ][ column ];
    }
  }

  // insert new display
  if ( !newDisplay ) {
    newDisplay = new DummyDisplay( this, &mSharedSettings );
    mDisplayList[ row ][ column ] = newDisplay;
  }  else {
    mDisplayList[ row ][ column ] = newDisplay;
    connect( newDisplay, SIGNAL( showPopupMenu( KSGRD::SensorDisplay* ) ),
             SLOT( showPopupMenu( KSGRD::SensorDisplay* ) ) );
    newDisplay->setDeleteNotifier( this );
  }

  mGridLayout->addWidget( mDisplayList[ row ][ column ], row, column );
  if(mRows == 1 && mColumns == 1) {  //if there's only item, the tab's title should be the widget's title
    connect( newDisplay, SIGNAL(titleChanged(const QString&)), SLOT(setTitle(const QString&)));
    setTitle(newDisplay->title());
  }
  if ( isVisible() ) {
    mDisplayList[ row ][ column ]->show();
  }
}

void WorkSheet::removeDisplay( KSGRD::SensorDisplay *display )
{
  if ( !display )
    return;
  
  if(display == mLocalProcessController) 
    mLocalProcessController = NULL;

  for ( uint r = 0; r < mRows; ++r )
    for ( uint c = 0; c < mColumns; ++c )
      if ( mDisplayList[ r ][ c ] == display ) {
        replaceDisplay( r, c );
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
   * are no longer used to have a stretch factor of 0 and hence be
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

void WorkSheet::changeEvent( QEvent * event ) {
  if (event->type() == QEvent::LanguageChange) {
    setTitle(mTitle);  //retranslate
  }
}

void WorkSheet::setUpdateInterval( float secs)
{
  if(secs == 0)
    mTimer.stop();
  else {
    mTimer.setInterval(secs*1000);
    mTimer.start();
  }
}
float WorkSheet::updateInterval() const
{
  if(mTimer.isActive())
    return mTimer.interval()/1000.0;
  else
    return 0;
}

#include "WorkSheet.moc"
