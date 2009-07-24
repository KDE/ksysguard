/*
    KSysGuard, the KDE System Guard

  Copyright (c) 1999, 2000, 2001 Chris Schlaeger <cs@kde.org>

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

#include <math.h>
#include <stdlib.h>

#include <QtXml/qdom.h>
#include <QtGui/QLCDNumber>
#include <QtGui/QHBoxLayout>

#include <kdebug.h>

#include <ksgrd/SensorManager.h>
#include "StyleEngine.h"

#include "MultiMeter.h"
#include "MultiMeterSettings.h"

MultiMeter::MultiMeter(QWidget* parent, const QString& title, SharedSettings *workSheetSettings)
  : KSGRD::SensorDisplay(parent, title, workSheetSettings)
{
  setShowUnit( true );
  mLowerLimit = mUpperLimit = 0.0;
  mLowerLimitActive = mUpperLimitActive = false;

  mIsFloat = false;

  mNormalDigitColor = KSGRD::Style->firstForegroundColor();
  mAlarmDigitColor = KSGRD::Style->alarmColor();
  QLayout *layout = new QHBoxLayout(this);
  mLcd = new QLCDNumber( this );
  layout->addWidget(mLcd);

  mLcd->setFrameStyle( QFrame::NoFrame );
  mLcd->setSegmentStyle( QLCDNumber::Filled );
  setDigitColor( KSGRD::Style->firstForegroundColor() );
  mLcd->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

  setBackgroundColor( KSGRD::Style->backgroundColor() );

  /* All RMB clicks to the mLcd widget will be handled by
   * SensorDisplay::eventFilter. */
  mLcd->installEventFilter( this );

  setPlotterWidget( mLcd );

  setMinimumSize( 5, 5 );
}

bool MultiMeter::addSensor(const QString& hostName, const QString& sensorName,
          const QString& sensorType, const QString& title)
{
  if ( (sensorType != "integer" && sensorType != "float") || sensorCount() > 0)
    return false;

  mIsFloat = (sensorType == "float");
  mLcd->setSmallDecimalPoint( mIsFloat );

  SensorDisplay::addSensor(hostName, sensorName,sensorType, title);

  /* To differentiate between answers from value requests and info
   * requests we use 100 for info requests. */
  sendRequest(hostName, sensorName + '?', 100);

  mLcd->setToolTip( QString("%1:%2").arg(hostName).arg(sensorName));

  return true;
}

void MultiMeter::answerReceived(int id, const QList<QByteArray>& answerlist)
{
  /* We received something, so the sensor is probably ok. */
  sensorError(id, false);
  QByteArray answer;
  if(!answerlist.isEmpty()) answer = answerlist[0];

  if (id == 100)
  {
    KSGRD::SensorIntegerInfo info(answer);
    setUnit(KSGRD::SensorMgr->translateUnit(info.unit()));
  }
  else
  {
    double val = answer.toDouble();

    int digits = 1;
    if (qAbs(val) >= 1) {
      digits = (int) log10(qAbs(val)) + 1;
    }
    if (mIsFloat) {
      //Show two digits after the decimal point
      digits += 3;
    }
    if (val < 0) {
      //Add a digit for the negative sign
      digits += 1;
    }

    mLcd->setNumDigits(qMin(15,digits));

    mLcd->display(val);
    if (mLowerLimitActive && val < mLowerLimit)
    {
      setDigitColor(mAlarmDigitColor);
    }
    else if (mUpperLimitActive && val > mUpperLimit)
    {
      setDigitColor(mAlarmDigitColor);
    }
    else
      setDigitColor(mNormalDigitColor);
  }
}

bool MultiMeter::restoreSettings(QDomElement& element)
{
  mLowerLimitActive = element.attribute("lowerLimitActive").toInt();
  mLowerLimit = element.attribute("lowerLimit").toDouble();
  mUpperLimitActive = element.attribute("upperLimitActive").toInt();
  mUpperLimit = element.attribute("upperLimit").toDouble();

  mNormalDigitColor = restoreColor(element, "normalDigitColor",
            KSGRD::Style->firstForegroundColor());
  mAlarmDigitColor = restoreColor(element, "mAlarmDigitColor",
            KSGRD::Style->alarmColor());
  setBackgroundColor(restoreColor(element, "backgroundColor",
            KSGRD::Style->backgroundColor()));

  addSensor(element.attribute("hostName"), element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "integer" : element.attribute("sensorType")), "");

  SensorDisplay::restoreSettings(element);

  return true;
}

bool MultiMeter::saveSettings(QDomDocument& doc, QDomElement& element)
{
  if(sensorCount() > 0) {
    element.setAttribute("hostName", sensor(0)->hostName());
    element.setAttribute("sensorName", sensor(0)->name());
    element.setAttribute("sensorType", sensor(0)->type());
  }
  element.setAttribute("showUnit", showUnit());
  element.setAttribute("lowerLimitActive", (int) mLowerLimitActive);
  element.setAttribute("lowerLimit", mLowerLimit);
  element.setAttribute("upperLimitActive", (int) mUpperLimitActive);
  element.setAttribute("upperLimit", mUpperLimit);

  saveColor(element, "normalDigitColor", mNormalDigitColor);
  saveColor(element, "mAlarmDigitColor", mAlarmDigitColor);
  saveColor(element, "backgroundColor", mBackgroundColor);

  SensorDisplay::saveSettings(doc, element);

  return true;
}

void MultiMeter::configureSettings()
{
  MultiMeterSettings dlg( this );

  dlg.setTitle(title());
  dlg.setShowUnit(showUnit());
  dlg.setLowerLimitActive(mLowerLimitActive);
  dlg.setLowerLimit(mLowerLimit);
  dlg.setUpperLimitActive(mUpperLimitActive);
  dlg.setUpperLimit(mUpperLimit);
  dlg.setNormalDigitColor(mNormalDigitColor);
  dlg.setAlarmDigitColor(mAlarmDigitColor);
  dlg.setMeterBackgroundColor(mBackgroundColor);

  if ( dlg.exec() ) {
    setShowUnit( dlg.showUnit() );
    setTitle( dlg.title() );
    mLowerLimitActive = dlg.lowerLimitActive();
    mLowerLimit = dlg.lowerLimit();
    mUpperLimitActive = dlg.upperLimitActive();
    mUpperLimit = dlg.upperLimit();

    mNormalDigitColor = dlg.normalDigitColor();
    mAlarmDigitColor = dlg.alarmDigitColor();
    setBackgroundColor( dlg.meterBackgroundColor() );

    repaint();
  }
}

void MultiMeter::applyStyle()
{
  mNormalDigitColor = KSGRD::Style->firstForegroundColor();
  setBackgroundColor( KSGRD::Style->backgroundColor() );

  repaint();
}

void MultiMeter::setDigitColor( const QColor& color )
{
  QPalette palette = mLcd->palette();
  palette.setColor( QPalette::WindowText, color );
  mLcd->setPalette( palette );
}

void MultiMeter::setBackgroundColor( const QColor& color )
{
  mBackgroundColor = color;

  QPalette pal = mLcd->palette();
  pal.setColor( mLcd->backgroundRole(), mBackgroundColor );
  mLcd->setPalette( pal );
}

#include "MultiMeter.moc"
