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


#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kstandarddirs.h>

#include <ksgrd/SensorManager.h>
#include "StyleEngine.h"

#include "FancyPlotterSettings.h"

#include "FancyPlotter.h"
#include "FancyPlotterSensor.h"
#include "AggregateFancyPlotterSensor.h"

class SensorToAdd {
    public:
        QRegExp name;
        QString hostname;
        QString type;
        QList<QColor> colors;
        QString summationName;
};

class FancyPlotterLabel : public QWidget {
    public:
        FancyPlotterLabel(QWidget *parent) : QWidget(parent) {
            QBoxLayout *layout = new QHBoxLayout();
            label = new QLabel();
            layout->addWidget(label);
            value = new QLabel();
            layout->addWidget(value);
            layout->addStretch(1);
            setLayout(layout);
        }
        ~FancyPlotterLabel() {
            delete label;
            delete value;
        }
        void setLabel(const QString &name, const QColor &color, const QChar &indicatorSymbol) {
            labelName = name;
            changeLabel(color, indicatorSymbol);
        }
        void changeLabel(const QColor &color, const QChar &indicatorSymbol) {
            if ( kapp->layoutDirection() == Qt::RightToLeft )
                label->setText(QString("<qt>: ") + labelName + " <font color=\"" + color.name() + "\">" + indicatorSymbol + "</font>");
            else
                label->setText(QString("<qt><font color=\"") + color.name() + "\">" + indicatorSymbol + "</font> " + labelName + " :");
        }

        QString labelName;
        QLabel *label;
        QLabel *value;
};

FancyPlotter::FancyPlotter( QWidget* parent,
        const QString &title,
        SharedSettings *workSheetSettings)
: KSGRD::SensorDisplay( parent, title, workSheetSettings )
{
    mBeams = 0;
    mNumAccountedFor = 0;
    mSettingsDialog = 0;
    mSensorReportedMax = mSensorReportedMin = 0;
    mAllMatchingSensorUnit = false;

    //The unicode character 0x25CF is a big filled in circle.  We would prefer to use this in the tooltip.
    //However it's maybe possible that the font used to draw the tooltip won't have it.  So we fall back to a
    //"#" instead.
    mIndicatorSymbol = '#';
    QFontMetrics fm(QToolTip::font());
    if(fm.inFont(QChar(0x25CF)))
        mIndicatorSymbol = QChar(0x25CF);

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    mPlotter = new KSignalPlotter( sensorDataProvider, this );
    int axisTextWidth = fontMetrics().width(i18nc("Largest axis title", "99999 XXXX"));
    mPlotter->setMaxAxisTextWidth( axisTextWidth );
    mHeading = new QLabel(translatedTitle(), this);
    QFont headingFont;
    headingFont.setFamily("Sans Serif");
    headingFont.setWeight(QFont::Bold);
    headingFont.setPointSize(11);
    mHeading->setFont(headingFont);
    layout->addWidget(mHeading);
    layout->addWidget(mPlotter);

    /* Create a set of labels underneath the graph. */
    QBoxLayout *outerLabelLayout = new QHBoxLayout;
    outerLabelLayout->setSpacing(0);
    layout->addLayout(outerLabelLayout);

    /* create a spacer to fill up the space up to the start of the graph */
    outerLabelLayout->addItem(new QSpacerItem(axisTextWidth, 0, QSizePolicy::Preferred));

    mLabelLayout = new QHBoxLayout;
    outerLabelLayout->addLayout(mLabelLayout);
    QFont font;
    font.setPointSize( KSGRD::Style->fontSize() );
    mPlotter->setAxisFont( font );
    mPlotter->setAxisFontColor( Qt::black );

    mPlotter->setThinFrame(!workSheetSettings);

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
    if(mHeading) {
        mHeading->setText(translatedTitle());
    }
}

bool FancyPlotter::eventFilter( QObject* object, QEvent* event ) {	//virtual
    if(event->type() == QEvent::ToolTip)
    {
        setTooltip();
    }

    return SensorDisplay::eventFilter(object, event);
}

void FancyPlotter::configureSettings()
{
    if(mSettingsDialog)
        return;
    mSettingsDialog = new FancyPlotterSettings( this, mSharedSettings->locked );

    mSettingsDialog->setTitle( title() );
    mSettingsDialog->setRangeType( mRangeType );
    mSettingsDialog->setMinValue( mPlotter->minValue() );
    mSettingsDialog->setMaxValue( mPlotter->maxValue() );

    mSettingsDialog->setHorizontalScale( mPlotter->horizontalScale() );

    mSettingsDialog->setShowVerticalLines( mPlotter->showVerticalLines() );
    mSettingsDialog->setGridLinesColor( mPlotter->verticalLinesColor() );
    mSettingsDialog->setVerticalLinesDistance( mPlotter->verticalLinesDistance() );
    mSettingsDialog->setVerticalLinesScroll( mPlotter->verticalLinesScroll() );

    mSettingsDialog->setShowHorizontalLines( mPlotter->showHorizontalLines() );

    mSettingsDialog->setShowAxis( mPlotter->showAxis() );

    mSettingsDialog->setFontSize( mPlotter->axisFont().pointSize()  );
    mSettingsDialog->setFontColor( mPlotter->axisFontColor() );

    mSettingsDialog->setBackgroundColor( mPlotter->backgroundColor() );

    SensorModelEntry::List list;
    int listSize = sensorCount();
    for ( int i = 0; i < listSize; ++i ) {
        SensorModelEntry entry;
        FancyPlotterSensor* currentSensor = static_cast<FancyPlotterSensor *>(sensor(i));
        entry.setId( i );
        entry.setHostName( currentSensor->hostName() );
        entry.setSensorName( currentSensor->name() );
        entry.setUnit( currentSensor->unit() );
        entry.setStatus( currentSensor->isOk() ? i18n( "OK" ) : i18n( "Error" ) );
        entry.setColor( currentSensor->color());

        list.append( entry );
    }
    mSettingsDialog->setSensors( list );

    connect( mSettingsDialog, SIGNAL( applyClicked() ), this, SLOT( applySettings() ) );
    connect( mSettingsDialog, SIGNAL( okClicked() ), this, SLOT( applySettings() ) );
    connect( mSettingsDialog, SIGNAL( finished() ), this, SLOT( settingsFinished() ) );

    mSettingsDialog->show();
}

void FancyPlotter::settingsFinished()
{
    mSettingsDialog->delayedDestruct();
    mSettingsDialog = 0;
}

void FancyPlotter::applySettings() {
    setTitle( mSettingsDialog->title() );

    mRangeType = mSettingsDialog->rangeType();
    if ( mRangeType == FancyPlotterSettings::Auto)
        mPlotter->setUseAutoRange( true );
    else {
        mPlotter->setUseAutoRange( false );
        if (mSettingsDialog->rangeType() == FancyPlotterSettings::ManualUser)
            mPlotter->changeRange( mSettingsDialog->minValue(), mSettingsDialog->maxValue() );
    }

    if ( mPlotter->horizontalScale() != mSettingsDialog->horizontalScale() ) {
        mPlotter->setHorizontalScale( mSettingsDialog->horizontalScale() );
    }

    mPlotter->setShowVerticalLines( mSettingsDialog->showVerticalLines() );
    mPlotter->setVerticalLinesColor( mSettingsDialog->gridLinesColor() );
    mPlotter->setVerticalLinesDistance( mSettingsDialog->verticalLinesDistance() );
    mPlotter->setVerticalLinesScroll( mSettingsDialog->verticalLinesScroll() );

    mPlotter->setShowHorizontalLines( mSettingsDialog->showHorizontalLines() );
    mPlotter->setHorizontalLinesColor( mSettingsDialog->gridLinesColor() );

    mPlotter->setShowAxis( mSettingsDialog->showAxis() );

    QFont font;
    font.setPointSize( mSettingsDialog->fontSize() );
    mPlotter->setAxisFont( font );
    mPlotter->setAxisFontColor( mSettingsDialog->fontColor() );

    mPlotter->setBackgroundColor( mSettingsDialog->backgroundColor() );

    QList<int> deletedBeams = mSettingsDialog->deleted();
    for ( int i =0; i < deletedBeams.count(); ++i) {
        removeSensor(deletedBeams[i]);
    }
    mSettingsDialog->clearDeleted(); //We have deleted them, so clear the deleted

    reorderBeams(mSettingsDialog->order());
    mSettingsDialog->resetOrder(); //We have now reordered the sensors, so reset the order

    SensorModelEntry::List list = mSettingsDialog->sensors();

    int listSize = sensorCount();
    for ( int i = 0; i < listSize; ++i ) {
        updateSensorColor(i,list[ i ].color());
    }
    mPlotter->update();
}

void FancyPlotter::updateSensorColor(const int argIndex, QColor argColor)  {
    (static_cast<FancyPlotterSensor *>(sensor(argIndex)))->setColor(argColor);
    static_cast<FancyPlotterLabel *>((static_cast<QWidgetItem *>(mLabelLayout->itemAt(argIndex)))->widget())->changeLabel(argColor, mIndicatorSymbol);
}
void FancyPlotter::reorderBeams(const QList<int> & orderOfBeams)
{
    //Q_ASSERT(orderOfBeams.size() == mLabelLayout.size());  Commented out because it cause compile problems in some cases??
    //Reorder the labels underneath the graph
    QList<QLayoutItem *> labelsInOldOrder;
    while(!mLabelLayout->isEmpty())
        labelsInOldOrder.append(mLabelLayout->takeAt(0));

    for(int newIndex = 0; newIndex < orderOfBeams.count(); newIndex++) {
        int oldIndex = orderOfBeams.at(newIndex);
        mLabelLayout->addItem(labelsInOldOrder.at(oldIndex));
    }

    sensorDataProvider->reorderSensor(orderOfBeams);



}
void FancyPlotter::applyStyle()
{
    mPlotter->setVerticalLinesColor( KSGRD::Style->firstForegroundColor() );
    mPlotter->setHorizontalLinesColor( KSGRD::Style->secondForegroundColor() );
    mPlotter->setBackgroundColor( KSGRD::Style->backgroundColor() );
    QFont font = mPlotter->axisFont();
    font.setPointSize(KSGRD::Style->fontSize() );
    mPlotter->setAxisFont( font );
    int listSize = sensorCount();
    for ( int i = 0; i < listSize && (unsigned int)i < KSGRD::Style->numSensorColors(); ++i ) {
        updateSensorColor(i,KSGRD::Style->sensorColor( i ));
    }

    mPlotter->update();
}

bool FancyPlotter::addSensor( const QString &hostName, const QString &name,
        const QString &type, const QString &title )
{
    Q_UNUSED(title);
    //we can ignore the title here since we always override the title when we get the result from the info request
    return addSensor( hostName, name, type, KSGRD::Style->sensorColor( mBeams ),"","");
}
bool FancyPlotter::addSensor( const QString &hostName, const QString &name,
        const QString &type, const QColor &color,
        const QString &regexpName, const QString &summationName)
{
    if ( type != "integer" && type != "float" )
        return false;

    FancyPlotterSensor* sensorToAdd = new FancyPlotterSensor(name,summationName,hostName,type,regexpName,color);
    return addSensor(sensorToAdd);
}

bool FancyPlotter::addSensor( const QString &hostName, const QList<QString> &name,
        const QString &type, const QColor &color,
        const QString &regexpName, const QString &summationName)
{
    if ( type != "integer" && type != "float" )
        return false;
    AggregateFancyPlotterSensor* sensorToAdd = new AggregateFancyPlotterSensor(name,summationName,hostName,type,regexpName,color);
    return addSensor(sensorToAdd);
}

bool FancyPlotter::addSensor(FancyPlotterSensor* sensorToAdd)
{
    sensorDataProvider->addSensor(sensorToAdd);
    /* To differentiate between answers from value requests and info
     * requests we add 100 to the beam index for info requests. */
    const QList<QString> nameList = sensorToAdd->nameList();
    foreach (QString currentName, nameList)  {
        sendRequest(sensorToAdd->hostName(), currentName + '?', sensorCount() - 1 + 100);
    }


    /* Add a label for this beam */
    FancyPlotterLabel *label = new FancyPlotterLabel(this);
    mLabelLayout->addWidget(label);
    if (!sensorToAdd->summationName().isEmpty()) {
        label->setLabel(sensorToAdd->summationName(), sensorToAdd->color(), mIndicatorSymbol);
    }
    ++mBeams;

    return true;
}

bool FancyPlotter::removeSensor( uint pos )
{
    if ( pos >= (uint)sensorCount() ) {
        kDebug(1215) << "FancyPlotter::removeBeam: beamId out of range ("
            << pos << ")" << endl;
        return false;
    }


    QWidget *label = (static_cast<QWidgetItem *>(mLabelLayout->takeAt( pos )))->widget();
    mLabelLayout->removeWidget(label);
    delete label;


    SensorDisplay::removeSensor(pos);
    //rescale to take into account the removed sensor
    mPlotter->rescale();

    return true;
}

void FancyPlotter::setTooltip()
{
    QString tooltip = "<qt><p style='white-space:pre'>";

    QString lastValue;
    bool neednewline = false;

    int listSize = sensorCount();

    for ( int i = 0; i < listSize; ++i ) {
        FancyPlotterSensor *currentSensor = static_cast<FancyPlotterSensor *>(sensor(i));
        if(currentSensor->isAggregateSensor()) {
            tooltip += i18nc("%1 is what is being shown statistics for, like 'Memory', 'Swap', etc.", "<p><b>%1:</b><br>", currentSensor->summationName());
            neednewline = false;
        }



        QList<QString> currentNameList = currentSensor->nameList();
        QList<QString> currentTitleList = currentSensor->titleList();
        int nameListSize = currentNameList.size();
        for (int j = 0;j < nameListSize;++j)  {
            lastValue = calculateLastValueAsString(currentSensor, j);
            QString currentName = currentNameList.at(j);
            QString currentTitle = "";
            if (j < currentTitleList.size() )
                currentTitle = currentTitleList.at(j);
            QString description = currentTitle;

            if (currentTitle.isEmpty()) {
                description = currentName;
            }


            if (currentSensor->isLocalHost()) {
                tooltip += QString("%1%2 %3 (%4)").arg(neednewline ? "<br>" : "") .arg("<font color=\"" + currentSensor->color().name() + "\">" + mIndicatorSymbol + "</font>") .arg(i18n(description.toUtf8())) .arg(lastValue);

            } else {
                tooltip += QString("%1%2 %3:%4 (%5)").arg(neednewline ? "<br>" : "") .arg("<font color=\"" + currentSensor->color().name() + "\">" + mIndicatorSymbol + "</font>") .arg(currentSensor->hostName()) .arg(i18n(description.toUtf8())) .arg(
                        lastValue);
            }
            neednewline = true;
        }

    }
    mPlotter->setToolTip( tooltip );
}

void FancyPlotter::timerTick() //virtual
{
    static const QString lastValueString = i18nc("%1 and %2 are sensor's last and maximum value", "%1 of %2", "%1", "%2");

    mPlotter->updatePlot();

    if (isVisible()) {
        if (QToolTip::isVisible() && mPlotter->geometry().contains(mPlotter->mapFromGlobal( QCursor::pos() ))) {
            setTooltip();
            QToolTip::showText(QCursor::pos(), mPlotter->toolTip(), mPlotter);
        }
        QString lastValue;
        int listSize = sensorCount();
        for (int i = 0; i < listSize; ++i) {
            FancyPlotterSensor *currentSensor = static_cast<FancyPlotterSensor *> (sensor(i));
            lastValue = calculateLastValueAsString(currentSensor);
            double max = currentSensor->reportedMaxValue();
            if ( max != 0 && currentSensor->unit() != "%")
                lastValue = lastValueString.arg(lastValue, calculateLastValueAsString(currentSensor,max));
            static_cast<FancyPlotterLabel *> ((static_cast<QWidgetItem *> (mLabelLayout->itemAt(i)))->widget())->value->setText(lastValue);
        }
    }


    SensorDisplay::timerTick();
}

QString FancyPlotter::calculateLastValueAsString(const FancyPlotterSensor * sensor, int sensorIndex) const {
    QString lastValue;
    if (sensor->isOk()) {
        if (sensor->dataSize() > 0) {
            lastValue = calculateLastValueAsString(sensor,sensor->lastValue(sensorIndex));
        } else {
            lastValue = i18n("N/A");
        }
    } else {
        lastValue = i18n("Error");
    }
    return lastValue;
}

QString FancyPlotter::calculateLastValueAsString(const FancyPlotterSensor * sensor, double value) const {
    static const QString translatedPercentage =  i18nc("units in percentage", "%1%", "%1");
    int precision;
    QString sensorUnit = sensor->unit();
    double scaleDownValue = value / mPlotter->scaleDownBy();
    //calculate the precision depending on the value
    if (scaleDownValue >= 99.5)
        precision = 0;
    else if (scaleDownValue < 0.995)
        precision = 2;
    else
        precision = 1;

    QString lastValue = KGlobal::locale()->formatNumber(scaleDownValue, precision);

    //if all the unit match use the graph units otherwise use the individual sensor's unit
    if (mAllMatchingSensorUnit)
        lastValue = mPlotter->unit().subs(lastValue).toString();
    else if (sensorUnit == "%")
        lastValue = translatedPercentage.arg(lastValue);
    else if (sensorUnit != "")
        lastValue = i18nc("units", ("%1 " + sensor->unit()).toUtf8(), lastValue);

    return lastValue;
}

void FancyPlotter::plotterAxisScaleChanged()
{
    //Prevent this being called recursively
    disconnect(mPlotter, SIGNAL(axisScaleChanged()), this, SLOT(plotterAxisScaleChanged()));
    KLocalizedString unit;
    double value = mPlotter->maxValue();

    //check if the units in the sensor matches, if one is different then set no unit
    QString calculatedUnit = "";
    int numSensors = sensorCount();
    if (numSensors > 0)  {
        int i = 1;
        mAllMatchingSensorUnit = true;
        FancyPlotterSensor* currentSensor = static_cast<FancyPlotterSensor *>(sensor(0));
        calculatedUnit = currentSensor->unit();
        while (i < numSensors)  {
            if (calculatedUnit != (static_cast<FancyPlotterSensor *>(sensor(i++)))->unit())  {
                calculatedUnit = "";
                mAllMatchingSensorUnit = false;
                break;
            }
        }
    }

    if(calculatedUnit  == "KiB") {
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
    } else if(calculatedUnit == "KiB/s") {
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
    } else if(calculatedUnit == "%") {
        mPlotter->setScaleDownBy(1);
        unit = ki18nc("units", "%1%"); //the unit - percentage
    } else if(calculatedUnit.isEmpty()) {
        unit = ki18nc("unitless - just a number", "%1");
    } else {
#if 0  // the strings are here purely for translation
        NOOP_I18NC("units", "%1 1/s"); // the unit - 1 per second
        NOOP_I18NC("units", "%1 s");  // the unit - seconds
        NOOP_I18NC("units", "%1 MHz");  // the unit - frequency megahertz
#endif
        mPlotter->setScaleDownBy(1);
        //translate any others
        unit = ki18nc("units", ("%1 " + calculatedUnit).toUtf8());
    }
    mPlotter->setUnit(unit);
    //reconnect
    connect(mPlotter, SIGNAL(axisScaleChanged()), this, SLOT(plotterAxisScaleChanged()));
}
void FancyPlotter::answerReceived( int id, const QList<QByteArray> &answerlist )
{
    QByteArray answer;
    int sensorListSize = sensorCount();
    if(!answerlist.isEmpty()) answer = answerlist[0];
    if ( (uint)id < 100 ) {
        if (id < sensorListSize)  {
            //Make sure that we put the answer in the correct place.  It's index in the list should be equal to the sensor index.  This in turn will contain the beamId
            FancyPlotterSensor *currentSensor = static_cast<FancyPlotterSensor *>(sensor(id));
            double value = answer.toDouble();
            currentSensor->addData(value);
            /* We received something, so the sensor is probably ok. */
            sensorError( id, false );
        }
    } else if ( id >= 100 && id < 200 ) {
        int adjustedId = id - 100;
        if (adjustedId < sensorListSize)  {
            KSGRD::SensorFloatInfo info( answer );
            QString currentUnit = info.unit();
            if(currentUnit.toUpper() == "KB" || currentUnit.toUpper() == "KIB")
                currentUnit = "KiB";
            if(currentUnit.toUpper() == "KB/S" || currentUnit.toUpper() == "KIB/S")
                currentUnit = "KiB/s";

            mSensorReportedMax = qMax(mSensorReportedMax, info.max());
            mSensorReportedMin = qMin(mSensorReportedMin, info.min());
            if (mRangeType == FancyPlotterSettings::ManualReported) {
                mPlotter->changeRange( mSensorReportedMin, mSensorReportedMax );
            }
            FancyPlotterSensor *currentSensor = static_cast<FancyPlotterSensor *>(sensor(adjustedId));
            currentSensor->setUnit(currentUnit);
            currentSensor->addTitle( info.name() );
            currentSensor->putReportedMaxValue(info.max());
            plotterAxisScaleChanged();

            QString summationName = currentSensor->summationName();
            if(summationName.isEmpty())
                static_cast<FancyPlotterLabel *>((static_cast<QWidgetItem *>(mLabelLayout->itemAt(adjustedId)))->widget())->setLabel(info.name(), currentSensor->color(), mIndicatorSymbol);
        }
    } else if (id == 200) {
        /* FIXME This doesn't check the host!  */
        if (!mSensorsToAdd.isEmpty()) {
            foreach(SensorToAdd *sensor, mSensorsToAdd)  {
                QList<QString> matchingSensorNameList;
                QColor color;
                bool isSummationNameEmpty = sensor->summationName.isEmpty();
                for (int i = 0; i < answerlist.count() && !answerlist[i].isEmpty(); ++i) {
                    QString sensorName = QString::fromUtf8(answerlist[i].split('\t')[0]);
                    if (sensor->name.exactMatch(sensorName)) {
                        //if summation name is empty we add a simple sensor right away
                        if (isSummationNameEmpty)  {
                            if (!sensor->colors.isEmpty())
                                color = sensor->colors.takeFirst();
                            else if (KSGRD::Style->numSensorColors() != 0)
                                color = KSGRD::Style->sensorColor(mBeams % KSGRD::Style->numSensorColors());
                            addSensor(sensor->hostname, sensorName, (sensor->type.isEmpty()) ? "float" : sensor->type, color, sensor->name.pattern(), sensor->summationName);
                        } else  {
                            matchingSensorNameList.append(sensorName);
                        }
                    }
                }

                //if the summation name is not empty then we add an sensor name list
                if (!isSummationNameEmpty && !matchingSensorNameList.isEmpty()) {
                    if (!sensor->colors.isEmpty())
                        color = sensor->colors.takeFirst();
                    else if (KSGRD::Style->numSensorColors() != 0)
                        color = KSGRD::Style->sensorColor(mBeams % KSGRD::Style->numSensorColors());
                    addSensor(sensor->hostname, matchingSensorNameList, (sensor->type.isEmpty()) ? "float" : sensor->type, color, sensor->name.pattern(), sensor->summationName);
                }

                delete sensor;
            }
            mSensorsToAdd.clear();
        }
    }
}

bool FancyPlotter::restoreSettings( QDomElement &element )
{
    /* autoRange was added after KDE 2.x and was brokenly emulated by
     * min == 0.0 and max == 0.0. Since we have to be able to read old
     * files as well we have to emulate the old behaviour as well. */
    double min = element.attribute( "min", "0.0" ).toDouble();
    double max = element.attribute( "max", "0.0" ).toDouble();
    mRangeType = FancyPlotterSettings::RangeType(element.attribute( "autoRange", min == 0.0 && max == 0.0 ? QString::number(FancyPlotterSettings::Auto) : QString::number(FancyPlotterSettings::ManualUser) ).toInt() );
    if (mRangeType == FancyPlotterSettings::Auto)
        mPlotter->setUseAutoRange( true );
    else {
        mPlotter->setUseAutoRange( false );
        mPlotter->changeRange( min, max);
    }
    // Do not restore the color settings from a previous version
    int version = element.attribute("version", "0").toInt();

    mPlotter->setShowVerticalLines( element.attribute( "vLines", "0" ).toUInt() );
    QColor vcolor = restoreColor( element, "vColor", mPlotter->verticalLinesColor() );
    mPlotter->setVerticalLinesColor( vcolor );
    mPlotter->setVerticalLinesDistance( element.attribute( "vDistance", "30" ).toUInt() );
    mPlotter->setVerticalLinesScroll( element.attribute( "vScroll", "0" ).toUInt() );
    mPlotter->setHorizontalScale( element.attribute( "hScale", "6" ).toUInt() );

    mPlotter->setShowHorizontalLines( element.attribute( "hLines", "1" ).toUInt() );
    mPlotter->setHorizontalLinesColor( restoreColor( element, "hColor",
                mPlotter->horizontalLinesColor() ) );

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

        mPlotter->setAxisFont( font );

        mPlotter->setAxisFontColor( restoreColor( element, "fontColor", Qt::black ) );  //make the default to be the same as the vertical line color
        mPlotter->setBackgroundColor( restoreColor( element, "bColor",
                    KSGRD::Style->backgroundColor() ) );
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
                      el.attribute( "sensorType" ) ), restoreColor( el, "color",
                      KSGRD::Style->sensorColor( mBeams ) ), QString(), el.attribute("summationName") );
    }

    SensorDisplay::restoreSettings( element );

    return true;
}

bool FancyPlotter::saveSettings( QDomDocument &doc, QDomElement &element)
{
    element.setAttribute( "min", mPlotter->minValue() );
    element.setAttribute( "max", mPlotter->maxValue() );
    element.setAttribute( "autoRange", mRangeType );
    element.setAttribute( "vLines", mPlotter->showVerticalLines() );
    saveColor( element, "vColor", mPlotter->verticalLinesColor() );
    element.setAttribute( "vDistance", mPlotter->verticalLinesDistance() );
    element.setAttribute( "vScroll", mPlotter->verticalLinesScroll() );

    element.setAttribute( "hScale", mPlotter->horizontalScale() );

    element.setAttribute( "hLines", mPlotter->showHorizontalLines() );
    saveColor( element, "hColor", mPlotter->horizontalLinesColor() );

    element.setAttribute( "svgBackground", mPlotter->svgBackground() );

    element.setAttribute( "version", 1 );
    element.setAttribute( "labels", mPlotter->showAxis() );
    saveColor ( element, "fontColor", mPlotter->axisFontColor());

    saveColor( element, "bColor", mPlotter->backgroundColor() );

    QHash<QString,QDomElement> hash;
    int listSize = sensorCount();
    for ( int i = 0; i < listSize; ++i ) {
        FancyPlotterSensor *currentSensor = static_cast<FancyPlotterSensor *>(sensor(i));


        QString regExpName = currentSensor->regexpName();
        if(!regExpName.isEmpty() && hash.contains( regExpName )) {
            QDomElement oldBeam = hash.value(regExpName);
            saveColorAppend( oldBeam, "color", currentSensor->color() );
        } else {
            QDomElement beam = doc.createElement( "beam" );
            element.appendChild( beam );
            beam.setAttribute( "hostName", currentSensor->hostName() );
            if(regExpName.isEmpty())
                beam.setAttribute( "sensorName", currentSensor->name() );
            else {
                beam.setAttribute( "regexpSensorName", currentSensor->regexpName() );
                hash[regExpName] = beam;
            }
            if(!currentSensor->summationName().isEmpty())
                beam.setAttribute( "summationName", currentSensor->summationName());
            beam.setAttribute( "sensorType", currentSensor->type() );
            saveColor( beam, "color", currentSensor->color() );
        }
    }
    SensorDisplay::saveSettings( doc, element );

    return true;
}

bool FancyPlotter::hasSettingsDialog() const
{
    return true;
}

#include "FancyPlotter.moc"
