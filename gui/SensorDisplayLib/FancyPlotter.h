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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#ifndef KSG_FANCYPLOTTER_H
#define KSG_FANCYPLOTTER_H

#include <kdialogbase.h>

#include <SensorDisplay.h>

#include "SignalPlotter.h"

class QListViewItem;
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
    FancyPlotter( QWidget* parent = 0, const char* name = 0,
                  const QString& title = QString::null, double min = 0,
                  double max = 100, bool noFrame = false );
    virtual ~FancyPlotter();

    void configureSettings();

    bool addSensor( const QString &hostName, const QString &name,
                    const QString &type, const QString &title );
    bool addSensor( const QString &hostName, const QString &name,
                    const QString &type, const QString &title,
                    const QColor &color );

    bool removeSensor( uint pos );

    virtual QSize sizeHint(void);

    virtual void answerReceived( int id, const QString &answer );

    virtual bool restoreSettings( QDomElement &element );
    virtual bool saveSettings( QDomDocument &doc, QDomElement &element,
                               bool save = true );

    virtual bool hasSettingsDialog() const;

  public slots:
    void applySettings();
    virtual void applyStyle();

  protected:
    virtual void resizeEvent( QResizeEvent* );

  private:
    uint mBeams;

    SignalPlotter* mPlotter;

    FancyPlotterSettings* mSettingsDialog;

    /**
      The sample buffer and the flags are needed to store the incoming
      samples for each beam until all samples of the period have been
      received. The flags variable is used to ensure that all samples have
      been received.
     */
    QValueList<double> mSampleBuf;
};

#endif
