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

#include <ksgrd/SensorManager.h>

#include "DancingBars.h"
#include "DummyDisplay.h"
#include "FancyPlotter.h"
#include "ListView.h"
#include "LogFile.h"
#include "MultiMeter.h"
#include "ProcessController.h"
#include "ProcessTable.h"
#include "SensorLogger.h"
#include "WorkSheet.h"
#include "WorkSheetSettings.h"

WorkSheet::WorkSheet( QWidget *parent )
  : QWidget( parent )
{
    mGridLayout = 0;
    mRows = mColumns = 0;
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setAcceptDrops( true );
}

    WorkSheet::WorkSheet( int rows, int columns, float interval, QWidget* parent )
: QWidget( parent)
{
    mGridLayout = 0;
    setUpdateInterval( interval );

    createGrid( rows, columns );

    // Initialize worksheet with dummy displays.
    const int numberOfElement = mRows*mColumns;
    for ( int i = 0; i < numberOfElement; i++ )
        replaceDisplay( i );

    mGridLayout->activate();

    setAcceptDrops( true );
}

WorkSheet::~WorkSheet()
{
    qDeleteAll(mDisplayList);
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
    if( interval  < 0 || interval > 100000 )  //make sure the interval is fairly sane
        interval = 0.5;

    setUpdateInterval(interval);

    mTitle = element.attribute( "title");
    mTranslatedTitle = mTitle.isEmpty() ? "" : i18n(mTitle.toUtf8());
    bool ok;
    mSharedSettings.locked = element.attribute( "locked" ).toUInt( &ok );
    if(!ok) mSharedSettings.locked = false;

    bool rowsOk, columnsOk;
    int rows = element.attribute( "rows" ).toInt( &rowsOk );
    int columns = element.attribute( "columns" ).toInt( &columnsOk );
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
        int row = element.attribute( "row" ).toInt();
        int column = element.attribute( "column" ).toInt();
        if ( row >= mRows || column >= mColumns) {
            kDebug(1215) << "Row or Column out of range (" << row << ", "
                << column << ")" << endl;
            return false;
        }
        replaceDisplay( row * mColumns + column, element );
    }

    // Fill empty cells with dummy displays
    for( int i = mDisplayList.count(); i < mRows * mColumns; i++)
        replaceDisplay(i);

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

    for( int i = 0; i < mDisplayList.count(); i++) {
        if ( QByteArray("DummyDisplay") != mDisplayList.at(i)->metaObject()->className()) {
            KSGRD::SensorDisplay* display = static_cast<KSGRD::SensorDisplay*>(mDisplayList.at(i));
            QDomElement element = doc.createElement( "display" );
            ws.appendChild( element );
            element.setAttribute( "row", i / mColumns  );
            element.setAttribute( "column", i % mColumns );
            element.setAttribute( "class", display->metaObject()->className() );

            display->saveSettings( doc, element );
        }
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
    int index;
    if ( !currentDisplay( &index ) )
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
    replaceDisplay( index, element );
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

KSGRD::SensorDisplay* WorkSheet::insertDisplay( DisplayType displayType, QString displayTitle, int index)
{
    KSGRD::SensorDisplay* newDisplay = 0;
    switch(displayType) {
        case DisplayDummy: 
            newDisplay = new DummyDisplay( this, &mSharedSettings );
            break;
        case DisplayFancyPlotter:
            newDisplay = new FancyPlotter( this, displayTitle, &mSharedSettings );
            break;
        case DisplayMultiMeter:
            newDisplay = new MultiMeter( this, displayTitle, &mSharedSettings);
            break;
        case DisplayDancingBars: 
            newDisplay = new DancingBars( this, displayTitle, &mSharedSettings);
            break;
        case DisplaySensorLogger:
            newDisplay = new SensorLogger( this, displayTitle, &mSharedSettings);
            break;
        case DisplayListView:
            newDisplay = new ListView( this, displayTitle, &mSharedSettings);
            break;
        case DisplayLogFile:
            newDisplay = new LogFile( this, displayTitle, &mSharedSettings );
            break;
        case DisplayProcessControllerRemote:
            newDisplay = new ProcessController(this, &mSharedSettings);
            break;
        case DisplayProcessControllerLocal:
            Q_ASSERT(sLocalProcessController);
            if (!sLocalProcessController->parentWidget())
                newDisplay = sLocalProcessController;
            else
                newDisplay = new ProcessController(this, &mSharedSettings);
            break;
        default:
            Q_ASSERT(false);
            return NULL;
    }
    newDisplay->applyStyle();
    connect(&mTimer, SIGNAL( timeout()), newDisplay, SLOT( timerTick()));
    replaceDisplay( index, newDisplay );
    return newDisplay;
}

KSGRD::SensorDisplay *WorkSheet::addDisplay( const QString &hostName,
        const QString &sensorName,
        const QString &sensorType,
        const QString& sensorDescr,
        int index )
{
    KSGRD::SensorDisplay* display = mDisplayList.at( index);
    /* If the by 'row' and 'column' specified display is a QGroupBox dummy
     * display we replace the widget. Otherwise we just try to add
     * the new sensor to an existing display. */
    if ( display->metaObject()->className() == QByteArray( "DummyDisplay" ) ) {
        DisplayType displayType = DisplayDummy;
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
                displayType = DisplayFancyPlotter;
            else if (execed == a2)
                displayType = DisplayMultiMeter;
            else if (execed == a3)
                displayType = DisplayDancingBars;
            else if (execed == a4)
                displayType = DisplaySensorLogger;
            else 
                return 0;
        } else if ( sensorType == "listview" ) {
            displayType = DisplayListView;
        }
        else if ( sensorType == "logfile" ) {
            displayType = DisplayLogFile;
        }
        else if ( sensorType == "sensorlogger" ) {
            displayType = DisplaySensorLogger;
        }
        else if ( sensorType == "table" ) {
            if(hostName.isEmpty() || hostName == "localhost")
                displayType = DisplayProcessControllerLocal;
            else
                displayType = DisplayProcessControllerRemote;
        }
        else {
            kDebug(1215) << "Unknown sensor type: " <<  sensorType;
            return 0;
        }
        display = insertDisplay(displayType, sensorDescr, index);
    }

    display->addSensor( hostName, sensorName, sensorType, sensorDescr );

    return display;
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
            if(mRows == 1 && mColumns == 1) {
                mDisplayList.first()->setTitle(dlg.sheetTitle());
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
    for ( int i = 0; i < mDisplayList.count(); i++ )
        mDisplayList[i]->applyStyle();
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
    for ( int i = 0; i < mDisplayList.count(); i++ ) {
        const QRect widgetRect = QRect( mDisplayList[i]->mapToGlobal( QPoint( 0, 0 ) ),
                mDisplayList[i]->size() );

        if ( widgetRect.contains( globalPos ) ) {
            QByteArray widgetType = mDisplayList[i]->metaObject()->className();
            if(widgetType == "MultiMeter" || widgetType == "ProcessController" || widgetType == "table")
                event->ignore(widgetRect);
            else if(widgetType != "Dummy")
                event->accept(widgetRect);
            return;
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
    for ( int i = 0; i < mDisplayList.count(); i++ ) {
        const QSize displaySize = mDisplayList[i]->size();

        const QPoint displayPoint( displaySize.width(), displaySize.height() );

        const QRect widgetRect = QRect( mDisplayList[i]->mapToGlobal( QPoint( 0, 0 ) ),
                mDisplayList[i]->mapToGlobal( displayPoint ) );


        if ( widgetRect.contains( globalPos ) ) {
            addDisplay( hostName, sensorName, sensorType, sensorDescr, i );
            return;
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
        if ( KMessageBox::warningContinueCancel( this, i18n( "Remove this display?" ),
                    i18n("Remove Display"), KStandardGuiItem::del() )
                == KMessageBox::Continue ) {
            KSGRD::SensorDisplay::DeleteEvent *event = static_cast<KSGRD::SensorDisplay::DeleteEvent*>( e );
            removeDisplay( event->display() );

            return true;
        }
    }

    return QWidget::event( e );
}

bool WorkSheet::replaceDisplay( int index, QDomElement& element )
{
    QString classType = element.attribute( "class" );
    QString hostName = element.attribute( "hostName" );
    DisplayType displayType = DisplayDummy;
    KSGRD::SensorDisplay* newDisplay;

    if ( classType == "FancyPlotter" )
        displayType = DisplayFancyPlotter;
    else if ( classType == "MultiMeter" )
        displayType = DisplayMultiMeter;
    else if ( classType == "DancingBars" )
        displayType = DisplayDancingBars;
    else if ( classType == "ListView" )
        displayType = DisplayListView;
    else if ( classType == "LogFile" )
        displayType = DisplayLogFile;
    else if ( classType == "SensorLogger" )
        displayType = DisplaySensorLogger;
    else if ( classType == "ProcessController" ) {
        if(hostName.isEmpty() || hostName == "localhost")
            displayType = DisplayProcessControllerLocal;
        else
            displayType = DisplayProcessControllerRemote;
    } else {
        kDebug(1215) << "Unknown class " <<  classType;
        return false;
    }

    newDisplay = insertDisplay(displayType, i18n("Dummy"), index);

    // load display specific settings
    if ( !newDisplay->restoreSettings( element ) )
        return false;

    return true;
}


void WorkSheet::replaceDisplay( int index, KSGRD::SensorDisplay* newDisplay )
{
    if ( !newDisplay )
        newDisplay = new DummyDisplay( this, &mSharedSettings );

    //Fill up with NULLs
    while( mDisplayList.count() < index )
        replaceDisplay(mDisplayList.count());

    if( mDisplayList.count() == index) {
        mDisplayList.append(newDisplay);
    } else {
        // remove the old display && sensor frame at this location
        if( mDisplayList[ index ] && mDisplayList[ index ] != sLocalProcessController )
            delete mDisplayList[ index ];
        mDisplayList[index] = newDisplay;
    }

    if( mDisplayList[ index ]->metaObject()->className() != QByteArray( "DummyDisplay" ) ) {
        connect( newDisplay, SIGNAL( showPopupMenu( KSGRD::SensorDisplay* ) ),
                SLOT( showPopupMenu( KSGRD::SensorDisplay* ) ) );
        newDisplay->setDeleteNotifier( this );
    }

    mGridLayout->addWidget( mDisplayList[ index ], index / mColumns, index % mColumns );
    if(mRows == 1 && mColumns == 1) {  //if there's only item, the tab's title should be the widget's title
        connect( newDisplay, SIGNAL(titleChanged(const QString&)), SLOT(setTitle(const QString&)));
        setTitle(newDisplay->title());
    }
    if ( isVisible() )
        mDisplayList[ index ]->show();
}

void WorkSheet::refreshSheet()
{
    for (int i = 0; i < mDisplayList.count(); i++)
        mDisplayList[i]->timerTick();
}

void WorkSheet::removeDisplay( KSGRD::SensorDisplay *display )
{
    if ( !display )
        return;

    for (int i = 0; i < mDisplayList.count(); i++)
        if ( mDisplayList[ i ] == display ) {
            replaceDisplay( i );
            return;
        }
}

void WorkSheet::collectHosts( QStringList &list )
{
    for (int i = 0; i < mDisplayList.count(); i++)
        if ( mDisplayList[ i ]->metaObject()->className() != QByteArray( "DummyDisplay" ) )
            ((KSGRD::SensorDisplay*)mDisplayList[ i ])->hosts( list );
}

void WorkSheet::createGrid( int rows, int columns )
{
    mRows = rows;
    mColumns = columns;

    // create grid layout with specified dimensions
    mGridLayout = new QGridLayout( this );
    mGridLayout->setSpacing( 5 );

    /* set stretch factors for rows and columns */
    for ( int r = 0; r < mRows; ++r )
        mGridLayout->setRowStretch( r, 100 );
    for ( int c = 0; c < mColumns; ++c )
        mGridLayout->setColumnStretch( c, 100 );
}

void WorkSheet::resizeGrid( int newRows, int newColumns )
{
    /* delete any excess displays */
    while( mDisplayList.count() > newRows * newColumns)
        delete mDisplayList.takeLast();

    /* remove all the items from the grid */
    while (mGridLayout->takeAt(0) != 0) { }

    /* set stretch factors for new rows and columns (if any) */
    for ( int r = mRows; r < newRows; ++r )
        mGridLayout->setRowStretch( r, 100 );
    for ( int c = mColumns; c < newColumns; ++c )
        mGridLayout->setColumnStretch( c, 100 );

    /* Obviously Qt does not shrink the size of the QGridLayout
     * automatically.  So we simply force the rows and columns that
     * are no longer used to have a stretch factor of 0 and hence be
     * invisible. */
    for ( int r = newRows; r < mRows; ++r )
        mGridLayout->setRowStretch( r, 0 );
    for ( int c = newColumns; c < mColumns; ++c )
        mGridLayout->setColumnStretch( c, 0 );

    mRows = newRows;
    mColumns = newColumns;

    /* Readd all the items to the grid */
    for( int r = 0; r < mRows; r++)
        for( int c = 0; c < mColumns && mDisplayList.count() > (r*mColumns + c); c++)
            mGridLayout->addWidget(mDisplayList[ r * mColumns + c], r, c);

    /* create new displays */
    for ( int i = mDisplayList.count(); i < mRows * mColumns; i++)
        replaceDisplay( i );

    fixTabOrder();

    mGridLayout->activate();
}

KSGRD::SensorDisplay *WorkSheet::currentDisplay( int * index )
{
    for(int i = 0; i < mDisplayList.count(); i++) {
        if ( mDisplayList[i]->hasFocus() ) {
            if ( index )
                *index = i;
            return ( mDisplayList[i] );
        }
    }
    return NULL;
}

void WorkSheet::fixTabOrder()
{
    for(int i = 0; i < mDisplayList.count()-1; i++)
        setTabOrder(mDisplayList[i], mDisplayList[i+1]);
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
