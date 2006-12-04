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

*/

#ifndef KSG_FANCYPLOTTER_H
#define KSG_FANCYPLOTTER_H

#include <SensorDisplay.h>
#include <QList>
#include <QResizeEvent>

#include "SignalPlotter.h"
#include "SharedSettings.h"

class FancyPlotterSettings;

class FPSensorProperties : public KSGRD::SensorProperties
{
  public:
    FPSensorProperties();
    FPSensorProperties( const QString &hostName, const QString &name,
                        const QString &type, const QString &description,
                        const QColor &color );
    ~FPSensorProperties();

    void setColor( const QColor &color );
    QColor color() const;

  private:
    QColor mColor;
};

class FancyPlotter : public KSGRD::SensorDisplay
{
  Q_OBJECT

  public:
    FancyPlotter( QWidget* parent, const QString& title, SharedSettings *workSheetSettings);
    virtual ~FancyPlotter();

    void configureSettings();

    bool addSensor( const QString &hostName, const QString &name,
                    const QString &type, const QString &title );
    bool addSensor( const QString &hostName, const QString &name,
                    const QString &type, const QString &title,
                    const QColor &color );

    bool removeSensor( uint pos );

    virtual QSize sizeHint(void) const;

    virtual void setTitle( const QString &title );

    virtual void answerReceived( int id, const QStringList &answerlist );

    virtual bool restoreSettings( QDomElement &element );
    virtual bool saveSettings( QDomDocument &doc, QDomElement &element );

    virtual bool hasSettingsDialog() const;

  public Q_SLOTS:
    virtual void applyStyle();
    void applySettings();

  protected:
    /** When we receive a timer event, draw the beams */
    virtual void timerEvent( QTimerEvent* );
    virtual bool eventFilter( QObject*, QEvent* );
    virtual void resizeEvent( QResizeEvent* );
    void setTooltip();

  private:
    uint mBeams;
    /** Number of beams we've recieved an answer from since asking last */
    uint mNumAccountedFor;

    KSignalPlotter* mPlotter;

    /**
      The sample buffer and the flags are needed to store the incoming
      samples for each beam until all samples of the period have been
      received. The flags variable is used to ensure that all samples have
      been received.
     */
    QList<double> mSampleBuf;

    FancyPlotterSettings* mSettingsDialog;
};

#endif
