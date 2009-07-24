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

#ifndef KSG_FANCYPLOTTER_H
#define KSG_FANCYPLOTTER_H

#include <SensorDisplay.h>
#include <QList>
#include <klocalizedstring.h>

#include "SignalPlotter.h"
#include "FancyPlotterSensor.h"

class FancyPlotterSettings;
class QLabel;
class SensorToAdd;
class FancyPlotterLabel;

class FancyPlotter: public KSGRD::SensorDisplay {
Q_OBJECT

public:
    FancyPlotter(QWidget* parent, const QString& title, SharedSettings *workSheetSettings);
    virtual ~FancyPlotter();

    void configureSettings();

    bool addSensor(FancyPlotterSensor* sensorToAdd);

    virtual bool addSensor(const QString &hostName, const QString &name, const QString &type, const QString &title);

    bool addSensor(const QString &hostName, const QString &name, const QString &type, const QColor &color, const QString &regexpName, const QString &summationName);

    bool addSensor(const QString &hostName, const QList<QString> &name, const QString &type, const QColor &color, const QString &regexpName, const QString &summationName);

    virtual void setTitle(const QString &title);

    virtual void answerReceived(int id, const QList<QByteArray> &answerlist);

    virtual bool restoreSettings(QDomElement &element);
    virtual bool saveSettings(QDomDocument &doc, QDomElement &element);

    virtual bool hasSettingsDialog() const;
public Q_SLOTS:
    virtual void applyStyle();

protected:
    /** When we receive a timer tick, draw the beams and request new information to update the beams*/
    virtual void timerTick();
    /*! \reimp */
    virtual bool eventFilter(QObject*, QEvent*);
    /*! \end_reimp */
    virtual void reorderBeams(const QList<int> & orderOfBeams);
    void setTooltip();

private Q_SLOTS:
    void plotterAxisScaleChanged();
    void settingsFinished();
    void applySettings();
private:

    static QString translated_LastValueString;

    int calculateLastValueAsString(FancyPlotterSensor *& sensor, QString & lastValue);
    bool removeSensor(uint pos);
    void updateSensorColor(const int argIndex, const QColor argColor);

    uint mBeams;
    /** Number of beams we've received an answer from since asking last */
    uint mNumAccountedFor;

    /** When we talk to the sensor, it tells us a range.  Record the max here.  equals 0 until we have an answer from it */
    double mSensorReportedMax;
    /** When we talk to the sensor, it tells us a range.  Record the min here.  equals 0 until we have an answer from it */
    double mSensorReportedMin;

    /** The widget that actually draws the beams */
    KSignalPlotter* mPlotter;

    FancyPlotterSettings* mSettingsDialog;
    QLabel *mHeading;

    QString mUnit;

    QList<SensorToAdd *> mSensorsToAdd;
    QBoxLayout *mLabelLayout;
    QChar mIndicatorSymbol;
};

inline int FancyPlotter::calculateLastValueAsString(FancyPlotterSensor *& sensor, QString & lastValue) {
    int precision = 0;
    if (sensor->isOk()) {
        if (sensor->dataSize() > 0) {
            if (sensor->unit() == mUnit) {
                precision = (sensor->isInteger() && mPlotter->scaleDownBy() == 1) ? 0 : -1;
                lastValue = mPlotter->valueAsString(sensor->lastValue(), precision);
            } else {
                precision = (sensor->isInteger()) ? 0 : -1;
                lastValue = KGlobal::locale()->formatNumber(sensor->lastValue(), precision);
                if (sensor->unit() == "%")
                    lastValue = i18nc("units", "%1%", lastValue);
                else if (sensor->unit() != "")
                    lastValue = i18nc("units", ("%1 " + sensor->unit()).toUtf8(), lastValue);
            }
        } else {
            lastValue = i18n("N/A");
        }

    } else {
        lastValue = i18n("Error");
    }
    return precision;

}

#endif
