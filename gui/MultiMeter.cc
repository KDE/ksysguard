/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999, 2000, 2001 Chris Schlaeger <cs@kde.org>
    
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

#include <math.h>
#include <stdlib.h>

#include <qcheckbox.h>
#include <qdom.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <knumvalidator.h>

#include <ksgrd/ColorPicker.h>
#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include "MultiMeter.moc"
#include "MultiMeterSettings.h"

MultiMeter::MultiMeter(QWidget* parent, const char* name,
				   const QString& title, double, double, bool nf)
	: KSGRD::SensorDisplay(parent, name, title)
{
	showUnit = true;
	lowerLimit = upperLimit = 0;
	lowerLimitActive = upperLimitActive = false;
	noFrame = nf;

	normalDigitColor = KSGRD::Style->getFgColor1();
	alarmDigitColor = KSGRD::Style->getAlarmColor();
	if (noFrame)
		lcd = new QLCDNumber(this, "meterLCD");
	else
		lcd = new QLCDNumber(frame, "meterLCD");
	Q_CHECK_PTR(lcd);
	lcd->setSegmentStyle(QLCDNumber::Filled);
	setDigitColor(KSGRD::Style->getBackgroundColor());
	lcd->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
					   QSizePolicy::Expanding, false));

	setBackgroundColor(KSGRD::Style->getBackgroundColor());
	/* All RMB clicks to the lcd widget will be handled by 
	 * SensorDisplay::eventFilter. */
	lcd->installEventFilter(this);

	registerPlotterWidget(lcd);

	setMinimumSize(16, 16);
	setModified(false);
}

bool
MultiMeter::addSensor(const QString& hostName, const QString& sensorName,
					const QString& sensorType, const QString& title)
{
	if (sensorType != "integer" && sensorType != "float")
		return (false);

	registerSensor(new KSGRD::SensorProperties(hostName, sensorName, sensorType, title));

	/* To differentiate between answers from value requests and info
	 * requests we use 100 for info requests. */
	sendRequest(hostName, sensorName + "?", 100);

	QToolTip::remove(lcd);
	QToolTip::add(lcd, QString("%1:%2").arg(hostName).arg(sensorName));

	setModified(true);
	return (true);
}

void
MultiMeter::answerReceived(int id, const QString& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);

	if (id == 100)
	{
		KSGRD::SensorIntegerInfo info(answer);
		setUnit(KSGRD::SensorMgr->translateUnit(info.getUnit()));
	}
	else
	{
		double val = answer.toDouble();
		int digits = (int) log10(val) + 1;

		if (noFrame)
			lcd->setNumDigits(2);
		else
		{
			if (digits > 5)
				lcd->setNumDigits(digits);
			else
				lcd->setNumDigits(5);
		}

		lcd->display(val);
		if (lowerLimitActive && val < lowerLimit)
		{
			setDigitColor(alarmDigitColor);	
		}
		else if (upperLimitActive && val > upperLimit)
		{
			setDigitColor(alarmDigitColor);
		}
		else
			setDigitColor(normalDigitColor);
	}
}

void
MultiMeter::resizeEvent(QResizeEvent*)
{
	if (noFrame)
		lcd->setGeometry(0, 0, width() - 1, height() - 1);
	else
		frame->setGeometry(0, 0, width(), height());
}

bool
MultiMeter::createFromDOM(QDomElement& element)
{
	lowerLimitActive = element.attribute("lowerLimitActive").toInt();
	lowerLimit = element.attribute("lowerLimit").toLong();
	upperLimitActive = element.attribute("upperLimitActive").toInt();
	upperLimit = element.attribute("upperLimit").toLong();

	normalDigitColor = restoreColorFromDOM(element, "normalDigitColor",
						KSGRD::Style->getFgColor1());
	alarmDigitColor = restoreColorFromDOM(element, "alarmDigitColor",
						KSGRD::Style->getAlarmColor());
	setBackgroundColor(restoreColorFromDOM(element, "backgroundColor",
						KSGRD::Style->getBackgroundColor()));

	addSensor(element.attribute("hostName"), element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "integer" : element.attribute("sensorType")), "");

	internCreateFromDOM(element);

	setModified(false);

	return (true);
}

bool
MultiMeter::addToDOM(QDomDocument& doc, QDomElement& element, bool save)
{
	element.setAttribute("hostName", sensors.at(0)->hostName);
	element.setAttribute("sensorName", sensors.at(0)->name);
	element.setAttribute("sensorType", sensors.at(0)->type);
	element.setAttribute("showUnit", (int) showUnit);
	element.setAttribute("lowerLimitActive", (int) lowerLimitActive);
	element.setAttribute("lowerLimit", (int) lowerLimit);
	element.setAttribute("upperLimitActive", (int) upperLimitActive);
	element.setAttribute("upperLimit", (int) upperLimit);

	addColorToDOM(element, "normalDigitColor", normalDigitColor);
	addColorToDOM(element, "alarmDigitColor", alarmDigitColor);
	addColorToDOM(element, "backgroundColor", lcd->backgroundColor());

	internAddToDOM(doc, element);

	if (save)
		setModified(false);

	return (true);
}

void
MultiMeter::settings()
{
	mms = new MultiMeterSettings(this, "MultiMeterSettings", true);
	Q_CHECK_PTR(mms);
	mms->title->setText(getTitle());
	mms->title->setFocus();
	mms->showUnit->setChecked(showUnit);
	mms->lowerLimitActive->setChecked(lowerLimitActive);
	mms->lowerLimit->setText(QString("%1").arg(lowerLimit));
	mms->lowerLimit->setValidator(new KFloatValidator(mms->lowerLimit));
	mms->upperLimitActive->setChecked(upperLimitActive);
	mms->upperLimit->setText(QString("%1").arg(upperLimit));
	mms->upperLimit->setValidator(new KFloatValidator(mms->upperLimit));
	mms->normalDigitColor->setColor(normalDigitColor);
	mms->alarmDigitColor->setColor(alarmDigitColor);
	mms->backgroundColor->setColor(lcd->backgroundColor());
	connect(mms->applyButton, SIGNAL(clicked()),
			this, SLOT(applySettings()));

	if (mms->exec())
		applySettings();

	delete mms;
	mms = 0;
}

void
MultiMeter::applySettings()
{
	showUnit = mms->showUnit->isChecked();
	setTitle(mms->title->text());
	lowerLimitActive = mms->lowerLimitActive->isChecked();
	lowerLimit = mms->lowerLimit->text().toDouble();
	upperLimitActive = mms->upperLimitActive->isChecked();
	upperLimit = mms->upperLimit->text().toDouble();

	normalDigitColor = mms->normalDigitColor->getColor();
	alarmDigitColor = mms->alarmDigitColor->getColor();
	setBackgroundColor(mms->backgroundColor->getColor());

	repaint();
	setModified(true);
}

void
MultiMeter::applyStyle()
{
	normalDigitColor = KSGRD::Style->getFgColor1();
	setBackgroundColor(KSGRD::Style->getBackgroundColor());
	repaint();
	setModified(true);
}

void
MultiMeter::setDigitColor(const QColor& col)
{
	QPalette p = lcd->palette();
	p.setColor(QColorGroup::Foreground, col);
	lcd->setPalette(p);
}

void
MultiMeter::setBackgroundColor(const QColor& col)
{
	lcd->setBackgroundColor(col);

	QPalette p = lcd->palette();
	p.setColor(QColorGroup::Light, col);
	p.setColor(QColorGroup::Dark, col);
	lcd->setPalette(p);
}
