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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

    $Id$
*/

#ifndef KSG_SENSORDISPLAY_H
#define KSG_SENSORDISPLAY_H

#include <qgroupbox.h>
#include <qlabel.h>
#include <qvaluelist.h>
#include <qwidget.h>

#include <knotifyclient.h>

#include <ksgrd/SensorClient.h>

#define NONE -1

class QDomDocument;
class QDomElement;

namespace KSGRD {

class SensorProperties;

/**
  This class is the base class for all displays for sensors. A
  display is any kind of widget that can display the value of one or
  more sensors in any form. It must be inherited by all displays that
  should be inserted into the work sheet.
 */
class SensorDisplay : public QWidget, public SensorClient
{
  Q_OBJECT

  public:
    /**
      Constructor.
     */
    SensorDisplay( QWidget *parent = 0, const char *name = 0, 
                   const QString& title = 0 );

    /**
      Destructor.
     */
    virtual ~SensorDisplay();

    /**
      Sets the title of the display.
     */
    void setTitle( const QString &title );

    /**
      Returns the title of the display.
     */
    QString title() const;

    /**
      Sets the unit of the display.
     */
    void setUnit( const QString &unit );

    /**
      Returns the unit of the display.
     */
    QString unit() const;

    /**
      Sets whether the unit string should be displayed at the top
      of the display frame.
     */
    void setShowUnit( bool value );

    /**
      Returns whether the unit string should be displayed at the top
      of the display frame. @see setShowUnit()
     */
    bool showUnit() const;

    /**
      Sets whether the update interval of the work sheet should be
      used instead of the one, set by @ref setUpdateInterval().
     */
    void setUseGlobalUpdateInterval( bool value );

    /**
      Returns whether the update interval of the work sheet should be
      used instead of the one, set by @ref setUpdateInterval().
      see @ref setUseGlobalUpdateInterval()
     */
    bool useGlobalUpdateInterval() const;

    /**
      Sets the update interval of the timer, which triggers the timer
      events. The state of the timer can be set with @ref setTimerOn().
     */
    void setUpdateInterval( uint interval );

    /**
      Returns the update interval.
     */
    uint updateInterval() const;

    /**
      This method appends all hosts of the display to @ref list.
     */
    void hosts( QStringList& list );

    /**
      Sets the widget on which the error icon can be drawn.
     */
    void setPlotterWidget( QWidget *plotter );

    /**
      Returns the widget on which the error icon can be drawn.
     */
    QWidget *plotterWidget() const;

    /**
      Add a sensor to the display.
      
      @param hostName The name of the host, the sensor belongs to.
      @param name The sensor name.
      @param type The type of the sensor.
      @param description A short description of the sensor.
     */
    virtual bool addSensor( const QString &hostName, const QString &name,
                            const QString &type, const QString &description );

    /**
      Removes the sensor from the display, that is at the position
      @ref pos of the intern sensor list.
     */
    virtual bool removeSensor( uint pos );

    /**
      This function is a wrapper function to SensorManager::sendRequest.
      It should be used by all SensorDisplay functions that need to send
      a request to a sensor since it performs an appropriate error
      handling by removing the display of necessary.
     */
    void sendRequest( const QString &hostName, const QString &cmd, int id );

    /**
      Raises the configure dialog to setup the update interval.
     */
    void configureUpdateInterval();

    /**
      Returns whether the display provides a settings dialog.
      This method should be reimplemented in the derived class.
     */
    virtual bool hasSettingsDialog() const;

    /**
      This method is called to raise the settings dialog of the
      display. It should be reimplemented in the derived class.
     */
    virtual void configureSettings();

    /**
      Reimplement this method to setup the display from config data.
     */
    virtual bool restoreSettings( QDomElement& );

    /**
      Reimplement this method to save the displays config data.
     */
    virtual bool saveSettings( QDomDocument&, QDomElement&, bool = true );

    /**
      Reimplement this method to catch error messages from the SensorManager.
      
      @param sensorId The unique id of the sensor.
      @param mode The mode: true = error, false = everthing ok
     */
    virtual void sensorError( int sensorId, bool mode );

    /**
      Normaly you shouldn't reimplement this methode
     */
    virtual void sensorLost( int reqId );

  public slots:
    /**
      If @ref value is true, this method starts the timer that triggers
      timer events. If @ref value is false, the timer is stopped.
     */
    void setTimerOn( bool value );

    /**
      Calling this method emits the @ref showPopupMenu() with this
      display as argument.
     */
    void rmbPressed();

    /**
      Sets whether the display is modified of not.
     */
    void setModified( bool modified );
    
    /**
      This method can be used to apply the new settings. Just connect
      the applyClicked() signal of your configuration dialog with this
      slot and reimplement it.
     */
    virtual void applySettings();

    /**
      This methid is called whenever the global style is changed.
      Reimplement it to apply the new style settings to the display.
     */
    virtual void applyStyle();

		
  signals:
    void showPopupMenu( KSGRD::SensorDisplay *display );
    void modified( bool modified );

  protected:
    virtual bool eventFilter( QObject*, QEvent* );
    virtual void focusInEvent( QFocusEvent* );
    virtual void focusOutEvent( QFocusEvent* );
    virtual void resizeEvent( QResizeEvent* );
    virtual void timerEvent( QTimerEvent* );

    void registerSensor( SensorProperties *sp );
    void unregisterSensor( uint pos );

    QColor restoreColor( QDomElement &element, const QString &attr,
                         const QColor& fallback );
    void saveColor( QDomElement &element, const QString &attr,
                    const QColor &color );

    virtual QString additionalWhatsThis();

    void setSensorOk( bool ok );

    bool modified() const;
    bool timerOn() const;

    QWidget *frame();

    void setNoFrame( bool value );
    bool noFrame() const;

    QPtrList<SensorProperties> &sensors();

  private:
    void updateWhatsThis();

    bool mShowUnit;
    bool mUseGlobalUpdateInterval;
    bool mModified;
    bool mNoFrame;

    int mTimerId;
    int mUpdateInterval;

    // The frame around the other widgets.
    QGroupBox* mFrame;

    QPtrList<SensorProperties> mSensors;

    QString mTitle;
    QString mUnit;

    QWidget* mErrorIndicator;
    QWidget* mPlotterWdg;
};

class SensorProperties
{
  public:
    SensorProperties();
    SensorProperties( const QString &hostName, const QString &name,
                      const QString &type, const QString &description );
    ~SensorProperties();

    void setHostName( const QString &hostName );
    QString hostName() const;

    void setName( const QString &name );
    QString name() const;

    void setType( const QString &type );
    QString type() const;

    void setDescription( const QString &description );
    QString description() const;

    void setUnit( const QString &unit );
    QString unit() const;

    void setIsOk( bool value );
    bool isOk() const;

  private:
    QString mHostName;
    QString mName;
    QString mType;
    QString mDescription;
    QString mUnit;

    /* This flag indicates whether the communication to the sensor is
     * ok or not. */
    bool mOk;
};

}

#endif
