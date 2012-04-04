/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>

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

#include <QtXml/qdom.h>
#include <QtGui/QImage>
#include <QtGui/QToolTip>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QFontInfo>
#include <QtGui/QResizeEvent>


#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <ksignalplotter.h>
#include <kstandarddirs.h>

#include <ksgrd/SensorManager.h>
#include "StyleEngine.h"

#include "FancyPlotterSettings.h"

#include "FancyPlotter.h"

class SensorToAdd {
  public:
    QRegExp name;
    QString hostname;
    QString type;
    QList<QColor> colors;
    QString summationName;
};

class FancyPlotterLabel : public QLabel {
  public:
    FancyPlotterLabel(QWidget *parent) : QLabel(parent) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        longHeadingWidth = 0;
        shortHeadingWidth = 0;
        textMargin = 0;
        setLayoutDirection(Qt::LeftToRight); //We do this because we organise the strings ourselves.. is this going to muck it up though for RTL languages?
    }
    ~FancyPlotterLabel() {
    }
    void setLabel(const QString &name, const QColor &color) {
        labelName = name;

        if(indicatorSymbol.isNull()) {
            if(fontMetrics().inFont(QChar(0x25CF)))
                indicatorSymbol = QChar(0x25CF);
            else
                indicatorSymbol = '#';
        }
        changeLabel(color);

    }
    void setValueText(const QString &value) {
        //value can have multiple strings, separated with the 0x9c character
        valueText = value.split(QChar(0x9c));
        resizeEvent(NULL);
        update();
    }
    virtual void resizeEvent( QResizeEvent * ) {
        QFontMetrics fm = fontMetrics();

        if(valueText.isEmpty()) {
            if(longHeadingWidth < width())
                setText(longHeadingText);
            else
                setText(shortHeadingText);
            return;
        }
        QString value = valueText.first();

        int textWidth = fm.boundingRect(value).width();
        if(textWidth + longHeadingWidth < width())
            setBothText(longHeadingText, value);
        else if(textWidth + shortHeadingWidth < width())
            setBothText(shortHeadingText, value);
        else {
            int valueTextCount = valueText.count();
            int i;
            for(i = 1; i < valueTextCount; ++i) {
                textWidth = fm.boundingRect(valueText.at(i)).width();
                if(textWidth + shortHeadingWidth <= width()) {
                    break;
                }
            }
            if(i < valueTextCount)
                setBothText(shortHeadingText, valueText.at(i));
            else
                setText(noHeadingText + valueText.last()); //This just sets the color of the text
        }
    }
    void changeLabel(const QColor &_color) {
        color = _color;

        if ( kapp->layoutDirection() == Qt::RightToLeft )
            longHeadingText = QString(": ") + labelName + " <font color=\"" + color.name() + "\">" + indicatorSymbol + "</font>";
        else
            longHeadingText = QString("<qt><font color=\"") + color.name() + "\">" + indicatorSymbol + "</font> " + labelName + " :";
        shortHeadingText = QString("<qt><font color=\"") + color.name() + "\">" + indicatorSymbol + "</font>";
        noHeadingText = QString("<qt><font color=\"") + color.name() + "\">";

        textMargin = fontMetrics().width('x') + margin()*2 + frameWidth()*2;
        longHeadingWidth = fontMetrics().boundingRect(labelName + " :" + indicatorSymbol + " x").width() + textMargin;
        shortHeadingWidth = fontMetrics().boundingRect(indicatorSymbol).width() + textMargin;
        setMinimumWidth(shortHeadingWidth);
        update();
    }
  private:
    void setBothText(const QString &heading, const QString & value) {
        if(QApplication::layoutDirection() == Qt::LeftToRight)
            setText(heading + ' ' + value);
        else
            setText("<qt>" + value + ' ' + heading);
    }
    int textMargin;
    QString longHeadingText;
    QString shortHeadingText;
    QString noHeadingText;
    int longHeadingWidth;
    int shortHeadingWidth;
    QList<QString> valueText;
    QString labelName;
    QColor color;
    static QChar indicatorSymbol;
};

QChar FancyPlotterLabel::indicatorSymbol;

FancyPlotter::FancyPlotter( QWidget* parent,
                            const QString &title,
                            SharedSettings *workSheetSettings)
  : KSGRD::SensorDisplay( parent, title, workSheetSettings )
{
    mBeams = 0;
    mSettingsDialog = 0;
    mSensorReportedMax = mSensorReportedMin = 0;
    mSensorManualMax = mSensorManualMin = 0;
    mUseManualRange = false;
    mNumAnswers = 0;
    mLabelsWidget = NULL;

    //The unicode character 0x25CF is a big filled in circle.  We would prefer to use this in the tooltip.
    //However it's maybe possible that the font used to draw the tooltip won't have it.  So we fall back to a 
    //"#" instead.
    QFontMetrics fm(QToolTip::font());
    if(fm.inFont(QChar(0x25CF)))
        mIndicatorSymbol = QChar(0x25CF);
    else
        mIndicatorSymbol = '#';

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    mPlotter = new KSignalPlotter( this );
    int axisTextWidth = fontMetrics().width(i18nc("Largest axis title", "99999 XXXX"));
    mPlotter->setMaxAxisTextWidth( axisTextWidth );
    mPlotter->setUseAutoRange( true );
    mHeading = new QLabel(translatedTitle(), this);
    QFont headingFont;
    headingFont.setFamily("Sans Serif");
    headingFont.setWeight(QFont::Bold);
    headingFont.setPointSize(11);
    mHeading->setFont(headingFont);
    layout->addWidget(mHeading);
    layout->addWidget(mPlotter);

    /* Create a set of labels underneath the graph. */
    mLabelsWidget = new QWidget;
    layout->addWidget(mLabelsWidget);
    QBoxLayout *outerLabelLayout = new QHBoxLayout(mLabelsWidget);
    outerLabelLayout->setSpacing(0);
    outerLabelLayout->setContentsMargins(0,0,0,0);

    /* create a spacer to fill up the space up to the start of the graph */
    outerLabelLayout->addItem(new QSpacerItem(axisTextWidth + 10, 0, QSizePolicy::Preferred));

    mLabelLayout = new QHBoxLayout;
    outerLabelLayout->addLayout(mLabelLayout);
    mLabelLayout->setContentsMargins(0,0,0,0);
    QFont font;
    font.setPointSize( KSGRD::Style->fontSize() );
    mPlotter->setFont( font );

    /* All RMB clicks to the mPlotter widget will be handled by
     * SensorDisplay::eventFilter. */
    mPlotter->installEventFilter( this );

    setPlotterWidget( mPlotter );
    connect(mPlotter, SIGNAL(axisScaleChanged()), this, SLOT(plotterAxisScaleChanged()));
    QDomElement emptyElement;
    restoreSettings(emptyElement);
}

FancyPlotter::~FancyPlotter()
{
}

void FancyPlotter::setTitle( const QString &title ) { //virtual
    KSGRD::SensorDisplay::setTitle( title );
    if(mHeading)
        mHeading->setText(translatedTitle());
}

bool FancyPlotter::eventFilter( QObject* object, QEvent* event ) {	//virtual
    if(event->type() == QEvent::ToolTip)
        setTooltip();
    return SensorDisplay::eventFilter(object, event);
}

void FancyPlotter::configureSettings()
{
    if(mSettingsDialog)
        return;
    mSettingsDialog = new FancyPlotterSettings( this, mSharedSettings->locked );

    mSettingsDialog->setTitle( title() );
    mSettingsDialog->setUseManualRange( mUseManualRange );
    if(mUseManualRange) {
        mSettingsDialog->setMinValue( mSensorManualMin );
        mSettingsDialog->setMaxValue( mSensorManualMax );
    } else {
        mSettingsDialog->setMinValue( mSensorReportedMin );
        mSettingsDialog->setMaxValue( mSensorReportedMax );
    }

    mSettingsDialog->setHorizontalScale( mPlotter->horizontalScale() );

    mSettingsDialog->setShowVerticalLines( mPlotter->showVerticalLines() );
    mSettingsDialog->setVerticalLinesDistance( mPlotter->verticalLinesDistance() );
    mSettingsDialog->setVerticalLinesScroll( mPlotter->verticalLinesScroll() );

    mSettingsDialog->setShowHorizontalLines( mPlotter->showHorizontalLines() );

    mSettingsDialog->setShowAxis( mPlotter->showAxis() );

    mSettingsDialog->setFontSize( mPlotter->font().pointSize()  );

    mSettingsDialog->setRangeUnits( mUnit );
    mSettingsDialog->setRangeUnits( mUnit );

    mSettingsDialog->setStackBeams( mPlotter->stackGraph() );

    bool hasIntegerRange = true;
    SensorModelEntry::List list;
    for ( int i = 0; i < (int)mBeams; ++i ) {
        FPSensorProperties *sensor = NULL;
        //find the first sensor for this beam, since one beam can have many sensors
        for ( int j = 0; j < sensors().count(); ++j ) {
            FPSensorProperties *sensor2 = static_cast<FPSensorProperties *>(sensors().at(j));
            if(sensor2->beamId == i)
                sensor = sensor2;
        }
        if(!sensor)
            return;
        SensorModelEntry entry;
        entry.setId( i );
        entry.setHostName( sensor->hostName() );
        entry.setSensorName( sensor->regExpName().isEmpty()?sensor->name():sensor->regExpName() );
        entry.setUnit( sensor->unit() );
        entry.setStatus( sensor->isOk() ? i18n( "OK" ) : i18n( "Error" ) );
        entry.setColor( mPlotter->beamColor( i ) );
        if(!sensor->isInteger)
            hasIntegerRange = false;
        list.append( entry );
    }
    mSettingsDialog->setSensors( list );
    mSettingsDialog->setHasIntegerRange( hasIntegerRange );

    connect( mSettingsDialog, SIGNAL(applyClicked()), this, SLOT(applySettings()) );
    connect( mSettingsDialog, SIGNAL(okClicked()), this, SLOT(applySettings()) );
    connect( mSettingsDialog, SIGNAL(finished()), this, SLOT(settingsFinished()) );

    mSettingsDialog->show();
}

void FancyPlotter::settingsFinished()
{
    mSettingsDialog->delayedDestruct();
    mSettingsDialog = 0;
}

void FancyPlotter::applySettings() {
    setTitle( mSettingsDialog->title() );

    mUseManualRange = mSettingsDialog->useManualRange();
    if(mUseManualRange) {
        mSensorManualMin = mSettingsDialog->minValue();
        mSensorManualMax = mSettingsDialog->maxValue();
        mPlotter->changeRange( mSettingsDialog->minValue(), mSettingsDialog->maxValue() );
    }
    else {
        mPlotter->changeRange( mSensorReportedMin, mSensorReportedMax );
    }

    if ( mPlotter->horizontalScale() != mSettingsDialog->horizontalScale() ) {
        mPlotter->setHorizontalScale( mSettingsDialog->horizontalScale() );
    }

    mPlotter->setShowVerticalLines( mSettingsDialog->showVerticalLines() );
    mPlotter->setVerticalLinesDistance( mSettingsDialog->verticalLinesDistance() );
    mPlotter->setVerticalLinesScroll( mSettingsDialog->verticalLinesScroll() );

    mPlotter->setShowHorizontalLines( mSettingsDialog->showHorizontalLines() );

    mPlotter->setShowAxis( mSettingsDialog->showAxis() );
    mPlotter->setStackGraph( mSettingsDialog->stackBeams() );

    QFont font;
    font.setPointSize( mSettingsDialog->fontSize() );
    mPlotter->setFont( font );

    QList<int> deletedBeams = mSettingsDialog->deleted();
    for ( int i =0; i < deletedBeams.count(); ++i) {
        removeBeam(deletedBeams[i]);
    }
    mSettingsDialog->clearDeleted(); //We have deleted them, so clear the deleted

    reorderBeams(mSettingsDialog->order());
    mSettingsDialog->resetOrder(); //We have now reordered the sensors, so reset the order

    SensorModelEntry::List list = mSettingsDialog->sensors();

    for( int i = 0; i < list.count(); i++)
        setBeamColor(i, list[i].color());
    mPlotter->update();
}

void FancyPlotter::resizeEvent( QResizeEvent* )
{
    bool showHeading = true;;
    bool showLabels = true;;

    if( height() < mLabelsWidget->sizeHint().height() + mHeading->sizeHint().height() + mPlotter->minimumHeight() )
        showHeading = false;
    if( height() < mLabelsWidget->sizeHint().height() + mPlotter->minimumHeight() )
        showLabels = false;
    mHeading->setVisible(showHeading);
    mLabelsWidget->setVisible(showLabels);

}

void FancyPlotter::reorderBeams(const QList<int> & orderOfBeams)
{
    //Q_ASSERT(orderOfBeams.size() == mLabelLayout.size());  Commented out because it cause compile problems in some cases??
    //Reorder the graph
    mPlotter->reorderBeams(orderOfBeams);
    //Reorder the labels underneath the graph
    QList<QLayoutItem *> labelsInOldOrder;
    while(!mLabelLayout->isEmpty())
        labelsInOldOrder.append(mLabelLayout->takeAt(0));

    for(int newIndex = 0; newIndex < orderOfBeams.count(); newIndex++) {
        int oldIndex = orderOfBeams.at(newIndex);
        mLabelLayout->addItem(labelsInOldOrder.at(oldIndex));
    }

    for ( int i = 0; i < sensors().count(); ++i ) {
        FPSensorProperties *sensor = static_cast<FPSensorProperties *>(sensors().at(i));
        for(int newIndex = 0; newIndex < orderOfBeams.count(); newIndex++) {
            int oldIndex = orderOfBeams.at(newIndex);
            if(oldIndex == sensor->beamId) {
                sensor->beamId = newIndex;
                break;
            }
        }
    }
}
void FancyPlotter::applyStyle()
{
    QFont font = mPlotter->font();
    font.setPointSize(KSGRD::Style->fontSize() );
    mPlotter->setFont( font );
    for ( int i = 0; i < mPlotter->numBeams() &&
            (unsigned int)i < KSGRD::Style->numSensorColors(); ++i ) {
        setBeamColor(i, KSGRD::Style->sensorColor(i));
    }

    mPlotter->update();
}
void FancyPlotter::setBeamColor(int i, const QColor &color)
{
        mPlotter->setBeamColor( i, color );
        static_cast<FancyPlotterLabel *>((static_cast<QWidgetItem *>(mLabelLayout->itemAt(i)))->widget())->changeLabel(color);
}
bool FancyPlotter::addSensor( const QString &hostName, const QString &name,
        const QString &type, const QString &title )
{
    return addSensor( hostName, name, type, title,
            KSGRD::Style->sensorColor( mBeams ), QString(), mBeams );
}

bool FancyPlotter::addSensor( const QString &hostName, const QString &name,
        const QString &type, const QString &title,
        const QColor &color, const QString &regexpName,
        int beamId, const QString & summationName)
{
    if ( type != "integer" && type != "float" )
        return false;


    registerSensor( new FPSensorProperties( hostName, name, type, title, color, regexpName, beamId, summationName ) );

    /* To differentiate between answers from value requests and info
     * requests we add 100 to the beam index for info requests. */
    sendRequest( hostName, name + '?', sensors().size() - 1 + 100 );

    if((int)mBeams == beamId) {
        mPlotter->addBeam( color );
        /* Add a label for this beam */
        FancyPlotterLabel *label = new FancyPlotterLabel(this);
        mLabelLayout->addWidget(label);
        if(!summationName.isEmpty()) {
            label->setLabel(summationName, mPlotter->beamColor(mBeams));
        }
        ++mBeams;
    }

    return true;
}

bool FancyPlotter::removeBeam( uint beamId )
{
    if ( beamId >= mBeams ) {
        kDebug(1215) << "FancyPlotter::removeBeam: beamId out of range ("
            << beamId << ")" << endl;
        return false;
    }

    mPlotter->removeBeam( beamId );
    --mBeams;
    QWidget *label = (static_cast<QWidgetItem *>(mLabelLayout->takeAt( beamId )))->widget();
    mLabelLayout->removeWidget(label);
    delete label;

    mSensorReportedMax = 0;
    mSensorReportedMin = 0;
    for ( int i = sensors().count()-1; i >= 0; --i ) {
        FPSensorProperties *sensor = static_cast<FPSensorProperties *>(sensors().at(i));

        if(sensor->beamId == (int)beamId)
            removeSensor( i );
        else {
            if(sensor->beamId > (int)beamId)
                sensor->beamId--;  //sensor pointer is no longer valid after removing the sensor
            mSensorReportedMax = qMax(mSensorReportedMax, sensor->maxValue);
            mSensorReportedMin = qMin(mSensorReportedMin, sensor->minValue);
        }
    }
    //change the plotter's range to the new maximum
    if ( !mUseManualRange )
        mPlotter->changeRange( mSensorReportedMin, mSensorReportedMax );
    else
        mPlotter->changeRange( mSensorManualMin, mSensorManualMax );

    //loop through the new sensors to find the new unit
    for ( int i = 0; i < sensors().count(); i++ ) {
        FPSensorProperties *sensor = static_cast<FPSensorProperties *>(sensors().at(i));
        if(i == 0)
            mUnit = sensor->unit();
        else if(mUnit != sensor->unit()) {
            mUnit = "";
            break;
        }
    }
    //adjust the scale to take into account the removed sensor
    plotterAxisScaleChanged();

    return true;
}

void FancyPlotter::setTooltip()
{
    QString tooltip = "<qt><p style='white-space:pre'>";

    QString description;
    QString lastValue;
    bool neednewline = false;
    bool showingSummationGroup = false;
    int beamId = -1;
    //Note that the number of beams can be less than the number of sensors, since some sensors
    //get added together for a beam.
    //For the tooltip, we show all the sensors
    for ( int i = 0; i < sensors().count(); ++i ) {
        FPSensorProperties *sensor = static_cast<FPSensorProperties *>(sensors().at(i));
        description = sensor->description();
        if(description.isEmpty())
            description = sensor->name();

        if(sensor->isOk()) {
            lastValue = KGlobal::locale()->formatNumber( sensor->lastValue, (sensor->isInteger)?0:-1 );
            if (sensor->unit() == "%")
                lastValue = i18nc("units", "%1%", lastValue);
            else if( !sensor->unit().isEmpty() )
                lastValue = i18nc("units", QString(QString("%1 ") + sensor->unit()).toUtf8(), lastValue);
        } else {
            lastValue = i18n("Error");
        }
        if (beamId != sensor->beamId) {
            if (!sensor->summationName.isEmpty()) {
                tooltip += i18nc("%1 is what is being shown statistics for, like 'Memory', 'Swap', etc.", "<p><b>%1:</b><br>", sensor->summationName);
                showingSummationGroup = true;
                neednewline = false;
            } else if (showingSummationGroup) {
                //When a summation group has finished, seperate the next sensor with a newline
                showingSummationGroup = false;
                tooltip += "<br>";
            }

        }
        beamId = sensor->beamId;

        if(sensor->isLocalhost()) {
            tooltip += QString( "%1%2 %3 (%4)" ).arg( neednewline  ? "<br>" : "")
                .arg("<font color=\"" + mPlotter->beamColor( beamId ).name() + "\">"+mIndicatorSymbol+"</font>")
                .arg( i18n(description.toUtf8()) )
                .arg( lastValue );

        } else {
            tooltip += QString( "%1%2 %3:%4 (%5)" ).arg( neednewline ? "<br>" : "" )
                .arg("<font color=\"" + mPlotter->beamColor( beamId ).name() + "\">"+mIndicatorSymbol+"</font>")
                .arg( sensor->hostName() )
                .arg( i18n(description.toUtf8()) )
                .arg( lastValue );
        }
        neednewline = true;
    }
    //  tooltip += "</td></tr></table>";
    mPlotter->setToolTip( tooltip );
}

void FancyPlotter::sendDataToPlotter( )
{
    if(!mSampleBuf.isEmpty() && mBeams != 0) {  
        if((uint)mSampleBuf.count() > mBeams) {
            mSampleBuf.clear();
            return; //ignore invalid results - can happen if a sensor is deleted
        }
        while((uint)mSampleBuf.count() < mBeams)
            mSampleBuf.append(mPlotter->lastValue(mSampleBuf.count())); //we might have sensors missing so set their values to the previously known value
        mPlotter->addSample( mSampleBuf );
        if(isVisible()) {
            if(QToolTip::isVisible() && (qApp->topLevelAt(QCursor::pos()) == window()) && mPlotter->geometry().contains(mPlotter->mapFromGlobal( QCursor::pos() ))) {
                setTooltip();
                QToolTip::showText(QCursor::pos(), mPlotter->toolTip(), mPlotter);
            }
            QString lastValue;
            int beamId = -1;
            for ( int i = 0; i < sensors().size(); ++i ) {
                FPSensorProperties *sensor = static_cast<FPSensorProperties *>(sensors().at(i));
                if(sensor->beamId == beamId)
                    continue;
                beamId = sensor->beamId;
                if(sensor->isOk() && mPlotter->numBeams() > beamId) {

                    int precision;
                    if(sensor->unit() == mUnit) {
                        precision = (sensor->isInteger && mPlotter->scaleDownBy() == 1)?0:-1;
                        lastValue = mPlotter->lastValueAsString(beamId, precision);
                    } else {
                        precision = (sensor->isInteger)?0:-1;
                        lastValue = KGlobal::locale()->formatNumber( mPlotter->lastValue(beamId), precision );
                        if (sensor->unit() == "%")
                            lastValue = i18nc("units", "%1%", lastValue);
                        else if( !sensor->unit().isEmpty() )  {
                            lastValue = i18nc("units", QString("%1 " + sensor->unit()).toUtf8(), lastValue);
                        }
                    }

                    if(sensor->maxValue != 0 && sensor->unit() != "%") {
                        //Use a multi length string incase we do not have enough room
                        lastValue = i18n("%1 of %2" "\xc2\x9c" "%1", lastValue, mPlotter->valueAsString(sensor->maxValue, precision) );
                    }
                } else {
                    lastValue = i18n("Error");
                }
                static_cast<FancyPlotterLabel *>((static_cast<QWidgetItem *>(mLabelLayout->itemAt(beamId)))->widget())->setValueText(lastValue);
            }
        }

    }
    mSampleBuf.clear();
}
void FancyPlotter::timerTick() //virtual
{
    if(mNumAnswers < sensors().count())
        sendDataToPlotter(); //we haven't received enough answers yet, but plot what we do have
    mNumAnswers = 0;
    SensorDisplay::timerTick();
}
void FancyPlotter::plotterAxisScaleChanged()
{
    //Prevent this being called recursively
    disconnect(mPlotter, SIGNAL(axisScaleChanged()), this, SLOT(plotterAxisScaleChanged()));
    KLocalizedString unit;
    double value = mPlotter->currentMaximumRangeValue();
    if(mUnit  == "KiB") {
        if(value >= 1024*1024*1024*0.7) {  //If it's over 0.7TiB, then set the scale to terabytes
            mPlotter->setScaleDownBy(1024*1024*1024);
            unit = ki18nc("units", "%1 TiB"); // the unit - terabytes
        } else if(value >= 1024*1024*0.7) {  //If it's over 0.7GiB, then set the scale to gigabytes
            mPlotter->setScaleDownBy(1024*1024);
            unit = ki18nc("units", "%1 GiB"); // the unit - gigabytes
        } else if(value > 1024) {
            mPlotter->setScaleDownBy(1024);
            unit = ki18nc("units", "%1 MiB"); // the unit - megabytes
        } else {
            mPlotter->setScaleDownBy(1);
            unit = ki18nc("units", "%1 KiB"); // the unit - kilobytes
        }
    } else if(mUnit == "KiB/s") {
        if(value >= 1024*1024*1024*0.7) {  //If it's over 0.7TiB, then set the scale to terabytes
            mPlotter->setScaleDownBy(1024*1024*1024);
            unit = ki18nc("units", "%1 TiB/s"); // the unit - terabytes per second
        } else if(value >= 1024*1024*0.7) {  //If it's over 0.7GiB, then set the scale to gigabytes
            mPlotter->setScaleDownBy(1024*1024);
            unit = ki18nc("units", "%1 GiB/s"); // the unit - gigabytes per second
        } else if(value > 1024) {
            mPlotter->setScaleDownBy(1024);
            unit = ki18nc("units", "%1 MiB/s"); // the unit - megabytes per second
        } else {
            mPlotter->setScaleDownBy(1);
            unit = ki18nc("units", "%1 KiB/s"); // the unit - kilobytes per second
        }
    } else if(mUnit == "%") {
        mPlotter->setScaleDownBy(1);
        unit = ki18nc("units", "%1%"); //the unit - percentage
    } else if(mUnit.isEmpty()) {
        unit = ki18nc("unitless - just a number", "%1");
    } else {
#if 0  // the strings are here purely for translation
        NOOP_I18NC("units", "%1 1/s"); // the unit - 1 per second
        NOOP_I18NC("units", "%1 s");  // the unit - seconds
        NOOP_I18NC("units", "%1 MHz");  // the unit - frequency megahertz
#endif
        mPlotter->setScaleDownBy(1);
        //translate any others
        unit = ki18nc("units", QString("%1 " + mUnit).toUtf8());
    }
    mPlotter->setUnit(unit);
    //reconnect
    connect(mPlotter, SIGNAL(axisScaleChanged()), this, SLOT(plotterAxisScaleChanged()));
}
void FancyPlotter::answerReceived( int id, const QList<QByteArray> &answerlist )
{
    QByteArray answer;

    if(!answerlist.isEmpty()) answer = answerlist[0];
    if ( (uint)id < 100 ) {
        //Make sure that we put the answer in the correct place.  Its index in the list should be equal to the sensor index.  This in turn will contain the beamId
        if(id >= sensors().count())
            return;  //just ignore if we get a result for an invalid sensor
        FPSensorProperties *sensor = static_cast<FPSensorProperties *>(sensors().at(id));
        int beamId = sensor->beamId;
        double value = answer.toDouble();
        while(beamId > mSampleBuf.count())
            mSampleBuf.append(0); //we might have sensors missing so set their values to zero

        if(beamId == mSampleBuf.count()) {
            mSampleBuf.append( value );
        } else {
            mSampleBuf[beamId] += value; //If we get two answers for the same beamid, we should add them together.  That's how the summation works
        }
        sensor->lastValue = value;
        /* We received something, so the sensor is probably ok. */
        sensorError( id, false );

        if(++mNumAnswers == sensors().count())
            sendDataToPlotter(); //we have received all the answers so start plotting
    } else if ( id >= 100 && id < 200 ) {
        if( (id - 100) >= sensors().count())
            return;  //just ignore if we get a result for an invalid sensor
        KSGRD::SensorFloatInfo info( answer );
        QString unit = info.unit();
        if(unit.toUpper() == "KB" || unit.toUpper() == "KIB")
            unit = "KiB";
        if(unit.toUpper() == "KB/S" || unit.toUpper() == "KIB/S")
            unit = "KiB/s";

        if(id == 100) //if we are the first sensor, just use that sensors units as the global unit
            mUnit = unit;
        else if(unit != mUnit)
            mUnit = ""; //if the units don't match, then set the units on the scale to empty, to avoid any confusion

        mSensorReportedMax = qMax(mSensorReportedMax, info.max());
        mSensorReportedMin = qMin(mSensorReportedMin, info.min());

        if ( !mUseManualRange )
            mPlotter->changeRange( mSensorReportedMin, mSensorReportedMax );
        plotterAxisScaleChanged();

        FPSensorProperties *sensor = static_cast<FPSensorProperties *>(sensors().at(id - 100));
        sensor->maxValue = info.max();
        sensor->minValue = info.min();
        sensor->setUnit( unit );
        sensor->setDescription( info.name() );

        QString summationName = sensor->summationName;
        int beamId = sensor->beamId;

        Q_ASSERT(beamId < mPlotter->numBeams());
        Q_ASSERT(beamId < mLabelLayout->count());

        if(summationName.isEmpty())
            static_cast<FancyPlotterLabel *>((static_cast<QWidgetItem *>(mLabelLayout->itemAt(beamId)))->widget())->setLabel(info.name(), mPlotter->beamColor(beamId));

    } else if( id == 200) {
        /* FIXME This doesn't check the host!  */
        if(!mSensorsToAdd.isEmpty())  {
            foreach(SensorToAdd *sensor, mSensorsToAdd) {
                int beamId = mBeams;  //Assign the next sensor to the next available beamId
                for ( int i = 0; i < answerlist.count(); ++i ) {
                    if ( answerlist[ i ].isEmpty() )
                        continue;
                    QString sensorName = QString::fromUtf8(answerlist[ i ].split('\t')[0]);
                    if(sensor->name.exactMatch(sensorName)) {
                        if(sensor->summationName.isEmpty())
                            beamId = mBeams; //If summationName is not empty then reuse the previous beamId.  In this way we can have multiple sensors with the same beamId, which can then be summed together
                        QColor color;
                        if(!sensor->colors.isEmpty() )
                            color = sensor->colors.takeFirst();
                        else if(KSGRD::Style->numSensorColors() != 0)
                            color = KSGRD::Style->sensorColor( beamId % KSGRD::Style->numSensorColors());
                        addSensor( sensor->hostname, sensorName,
                                (sensor->type.isEmpty()) ? "float" : sensor->type
                                , "", color, sensor->name.pattern(), beamId, sensor->summationName);
                    }
                }
            }
            qDeleteAll(mSensorsToAdd);
            mSensorsToAdd.clear();
        }
    }
}

bool FancyPlotter::restoreSettings( QDomElement &element )
{
    mUseManualRange = element.attribute( "manualRange", "0" ).toInt();

    if(mUseManualRange) {
        mSensorManualMax = element.attribute( "max" ).toDouble();
        mSensorManualMin = element.attribute( "min" ).toDouble();
        mPlotter->changeRange( mSensorManualMin, mSensorManualMax );
    } else {
        mPlotter->changeRange( mSensorReportedMin, mSensorReportedMax );
    }

    mPlotter->setUseAutoRange(element.attribute( "autoRange", "1" ).toInt());

    // Do not restore the color settings from a previous version
    int version = element.attribute("version", "0").toInt();

    mPlotter->setShowVerticalLines( element.attribute( "vLines", "0" ).toUInt() );
    mPlotter->setVerticalLinesDistance( element.attribute( "vDistance", "30" ).toUInt() );
    mPlotter->setVerticalLinesScroll( element.attribute( "vScroll", "0" ).toUInt() );
    mPlotter->setHorizontalScale( element.attribute( "hScale", "6" ).toUInt() );

    mPlotter->setShowHorizontalLines( element.attribute( "hLines", "1" ).toUInt() );
    mPlotter->setStackGraph( element.attribute("stacked", "0").toInt());

    QString filename = element.attribute( "svgBackground");
    if (!filename.isEmpty() && filename[0] == '/') {
        KStandardDirs* kstd = KGlobal::dirs();
        filename = kstd->findResource( "data", "ksysguard/" + filename);
    }
    mPlotter->setSvgBackground( filename );
    if(version >= 1) {
        mPlotter->setShowAxis( element.attribute( "labels", "1" ).toUInt() );
        uint fontsize = element.attribute( "fontSize", "0").toUInt();
        if(fontsize == 0) fontsize =  KSGRD::Style->fontSize();
        QFont font;
        font.setPointSize( fontsize );
        mPlotter->setFont( font );
    }
    QDomNodeList dnList = element.elementsByTagName( "beam" );
    for ( int i = 0; i < dnList.count(); ++i ) {
        QDomElement el = dnList.item( i ).toElement();
        if(el.hasAttribute("regexpSensorName")) {
            SensorToAdd *sensor = new SensorToAdd();
            sensor->name = QRegExp(el.attribute("regexpSensorName"));
            sensor->hostname = el.attribute( "hostName" );
            sensor->type = el.attribute( "sensorType" );
            sensor->summationName = el.attribute("summationName");
            QStringList colors = el.attribute("color").split(',');
            bool ok;
            foreach(const QString &color, colors) {
                int c = color.toUInt( &ok, 0 );
                if(ok) {
                    QColor col( (c & 0xff0000) >> 16, (c & 0xff00) >> 8, (c & 0xff), (c & 0xff000000) >> 24);
                    if(col.isValid()) {
                        if(col.alpha() == 0) col.setAlpha(255);
                        sensor->colors << col;
                    }
                    else
                        sensor->colors << KSGRD::Style->sensorColor( i );
                }
                else
                    sensor->colors << KSGRD::Style->sensorColor( i );
            }
            mSensorsToAdd.append(sensor);
            sendRequest( sensor->hostname, "monitors", 200 );
        } else
            addSensor( el.attribute( "hostName" ), el.attribute( "sensorName" ),
                    ( el.attribute( "sensorType" ).isEmpty() ? "float" :
                      el.attribute( "sensorType" ) ), "", restoreColor( el, "color",
                      KSGRD::Style->sensorColor( i ) ), QString(), mBeams, el.attribute("summationName") );
    }

    SensorDisplay::restoreSettings( element );

    return true;
}

bool FancyPlotter::saveSettings( QDomDocument &doc, QDomElement &element)
{
    element.setAttribute( "autoRange", mPlotter->useAutoRange() );

    element.setAttribute( "manualRange", mUseManualRange );
    if(mUseManualRange) {
        element.setAttribute( "min", mSensorManualMin );
        element.setAttribute( "max", mSensorManualMax );
    }


    element.setAttribute( "vLines", mPlotter->showVerticalLines() );
    element.setAttribute( "vDistance", mPlotter->verticalLinesDistance() );
    element.setAttribute( "vScroll", mPlotter->verticalLinesScroll() );

    element.setAttribute( "hScale", mPlotter->horizontalScale() );

    element.setAttribute( "hLines", mPlotter->showHorizontalLines() );

    element.setAttribute( "svgBackground", mPlotter->svgBackground() );
    element.setAttribute( "stacked", mPlotter->stackGraph() );

    element.setAttribute( "version", 1 );
    element.setAttribute( "labels", mPlotter->showAxis() );

    QHash<QString,QDomElement> hash;
    int beamId = -1;
    for ( int i = 0; i < sensors().size(); ++i ) {
        FPSensorProperties *sensor = static_cast<FPSensorProperties *>(sensors().at(i));
        if(sensor->beamId == beamId)
            continue;
        beamId = sensor->beamId;

        QString regExpName = sensor->regExpName();
        if(!regExpName.isEmpty() && hash.contains( regExpName )) {
            QDomElement oldBeam = hash.value(regExpName);
            saveColorAppend( oldBeam, "color", mPlotter->beamColor( beamId ) );
        } else {
            QDomElement beam = doc.createElement( "beam" );
            element.appendChild( beam );
            beam.setAttribute( "hostName", sensor->hostName() );
            if(regExpName.isEmpty())
                beam.setAttribute( "sensorName", sensor->name() );
            else {
                beam.setAttribute( "regexpSensorName", sensor->regExpName() );
                hash[regExpName] = beam;
            }
            if(!sensor->summationName.isEmpty())
                beam.setAttribute( "summationName", sensor->summationName);
            beam.setAttribute( "sensorType", sensor->type() );
            saveColor( beam, "color", mPlotter->beamColor( beamId ) );
        }
    }
    SensorDisplay::saveSettings( doc, element );

    return true;
}

bool FancyPlotter::hasSettingsDialog() const
{
    return true;
}

FPSensorProperties::FPSensorProperties()
{
}

FPSensorProperties::FPSensorProperties( const QString &hostName,
        const QString &name,
        const QString &type,
        const QString &description,
        const QColor &color,
        const QString &regexpName,
        int beamId_,
        const QString &summationName_ )
: KSGRD::SensorProperties( hostName, name, type, description),
    mColor( color )
{
    setRegExpName(regexpName);
    beamId = beamId_;
    summationName = summationName_;
    maxValue = 0;
    minValue = 0;
    lastValue = 0;
    isInteger = (type == "integer");
}

FPSensorProperties::~FPSensorProperties()
{
}

void FPSensorProperties::setColor( const QColor &color )
{
    mColor = color;
}

QColor FPSensorProperties::color() const
{
    return mColor;
}


#include "FancyPlotter.moc"
