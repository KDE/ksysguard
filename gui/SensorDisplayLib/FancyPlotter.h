/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy 
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KSG_FANCYPLOTTER_H
#define KSG_FANCYPLOTTER_H

#include <SensorDisplay.h>
#include <QList>
#include <klocalizedstring.h>

#include "SharedSettings.h"

class FancyPlotterSettings;
class QBoxLayout;
class QLabel;
class SensorToAdd;
class KSignalPlotter;

class FPSensorProperties : public KSGRD::SensorProperties
{
  public:
    FPSensorProperties();
    FPSensorProperties( const QString &hostName, const QString &name,
                        const QString &type, const QString &description,
                        const QColor &color, const QString &regexpName = QString(),
                        int beamId = -1, const QString &summationName = QString());
    ~FPSensorProperties();

    void setColor( const QColor &color );
    QColor color() const;
    int beamId;
    QString summationName;
    double maxValue;
    double minValue;
    double lastValue;
    bool isInteger;

  private:
    QColor mColor;
};

class FancyPlotter : public KSGRD::SensorDisplay
{
  Q_OBJECT

  public:
    FancyPlotter( QWidget* parent, const QString& title, SharedSettings *workSheetSettings);
    virtual ~FancyPlotter();

    void configureSettings() Q_DECL_OVERRIDE;

    bool addSensor( const QString &hostName, const QString &name,
                    const QString &type, const QString &title ) Q_DECL_OVERRIDE;
    bool addSensor( const QString &hostName, const QString &name,
                    const QString &type, const QString &title,
                    const QColor &color, const QString &regexpName = QString(), 
                    int sumToSensor = -1, const QString &summationName = QString());

    bool removeBeam( uint beamId );

    void setTitle( const QString &title ) Q_DECL_OVERRIDE;

    void answerReceived( int id, const QList<QByteArray> &answerlist ) Q_DECL_OVERRIDE;

    bool restoreSettings( QDomElement &element ) Q_DECL_OVERRIDE;
    bool saveSettings( QDomDocument &doc, QDomElement &element ) Q_DECL_OVERRIDE;

    bool hasSettingsDialog() const Q_DECL_OVERRIDE;
    void setBeamColor(int i, const QColor &color);

  public Q_SLOTS:
    void applyStyle() Q_DECL_OVERRIDE;
  private Q_SLOTS:
    void settingsFinished();
    void applySettings() Q_DECL_OVERRIDE;
    void plotterAxisScaleChanged();

  protected:
    /** When we receive a timer tick, draw the beams and request new information to update the beams*/
    void timerTick( ) Q_DECL_OVERRIDE;
    bool eventFilter( QObject*, QEvent* ) Q_DECL_OVERRIDE;
    virtual void reorderBeams(const QList<int> & orderOfBeams);
    void resizeEvent( QResizeEvent* ) Q_DECL_OVERRIDE;
    void setTooltip();

  private:
    void sendDataToPlotter();
    uint mBeams;
    
    int mNumAnswers;
    /** When we talk to the sensor, it tells us a range.  Record the max here.  equals 0 until we have an answer from it */
    double mSensorReportedMax;
    /** When we talk to the sensor, it tells us a range.  Record the min here.  equals 0 until we have an answer from it */
    double mSensorReportedMin;

    /** If mUseManualRange is true, this is maximum value given by the user. */
    double mSensorManualMax;
    /** If mUseManualRange is true, this is minimum value given by the user. */
    double mSensorManualMin;

    /** The widget that actually draws the beams */
    KSignalPlotter* mPlotter;

    /**
      The sample buffer and the flags are needed to store the incoming
      samples for each beam until all samples of the period have been
      received. The flags variable is used to ensure that all samples have
      been received.
     */
    QList<qreal> mSampleBuf;

    FancyPlotterSettings* mSettingsDialog;
    QLabel *mHeading;

    QString mUnit;

    QList<SensorToAdd *> mSensorsToAdd;
    QBoxLayout *mLabelLayout;
    QChar mIndicatorSymbol;

    /** True if we will override the values from ksysguardd with user-specified values. */
    bool mUseManualRange;
    QWidget *mLabelsWidget;
};

#endif
