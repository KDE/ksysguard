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
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void reorderBeams(const QList<int> & orderOfBeams);
    void setTooltip();

private Q_SLOTS:
    void plotterAxisScaleChanged();
    void settingsFinished();
    void applySettings();

private:
    /** Return the last value of @p sensor as string (e.g.  "31 MiB").
     *  If @p precisionP is not NULL, it is set the number of digits after the decimal place used.
     *  If the sensor is actually a sum of various other sensors, sensorIndex indicates which value to show.  0 indicates to return the sum.
     *
     *  @see FancyPlotterSensor::lastValue()
     */
    QString calculateLastValueAsString(const FancyPlotterSensor * sensor, int sensorIndex = 0, int *precisionP = NULL) const;
    bool removeSensor(uint pos);
    void updateSensorColor(const int argIndex, const QColor argColor);

    uint mBeams;
    /** Number of beams we've received an answer from since asking last */
    uint mNumAccountedFor;

    /** When we talk to the sensor, it tells us a range.  Record the max here.  Equals 0 until we have an answer from it */
    double mSensorReportedMax;
    /** When we talk to the sensor, it tells us a range.  Record the min here.  Equals 0 until we have an answer from it */
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

#endif
