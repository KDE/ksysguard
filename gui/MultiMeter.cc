/*
    KSysGuard, the KDE Task Manager and System Monitor
   
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

#include <math.h>
#include <stdlib.h>

#include <qgroupbox.h>
#include <qlcdnumber.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <knumvalidator.h>

#include "MultiMeterSettings.h"
#include "SensorManager.h"
#include "ColorPicker.h"
#include "MultiMeter.moc"

MultiMeter::MultiMeter(QWidget* parent, const char* name,
					   const QString& t, double, double)
	: SensorDisplay(parent, name)
{
	showUnit = TRUE;
	lowerLimit = upperLimit = 0;
	lowerLimitActive = upperLimitActive = FALSE;

	setTitle(t, unit);

	normalDigitColor = Qt::green;
	alarmDigitColor = Qt::red;
	lcd = new QLCDNumber(frame, "meterLCD");
	CHECK_PTR(lcd);
	lcd->setSegmentStyle(QLCDNumber::Filled);
	setDigitColor(Qt::black);
	lcd->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
								   QSizePolicy::Expanding, FALSE));

	KIconLoader iconLoader;
	QPixmap errorIcon = iconLoader.loadIcon("connect_creating", KIcon::Desktop,
											KIcon::SizeSmall);
	errorLabel = new QLabel(lcd);
	errorLabel->setPixmap(errorIcon);
	errorLabel->resize(errorIcon.size());
	errorLabel->move(2, 2);

	setBackgroundColor(Qt::black);
	/* All RMB clicks to the lcd widget will be handled by 
	 * SensorDisplay::eventFilter. */
	lcd->installEventFilter(this);

	setMinimumSize(16, 16);
	setModified(false);
}

bool
MultiMeter::addSensor(const QString& hostName, const QString& sensorName,
					  const QString& title)
{
	registerSensor(new SensorProperties(hostName, sensorName, title));

	/* To differentiate between answers from value requests and info
	 * requests we use 100 for info requests. */
	sendRequest(hostName, sensorName + "?", 100);

	setModified(TRUE);
	return (TRUE);
}

void
MultiMeter::answerReceived(int id, const QString& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);

	if (id == 100)
	{
		SensorIntegerInfo info(answer);
		setTitle(title, SensorMgr->translateUnit(info.getUnit()));
		timerOn();
	}
	else
	{
		double val = answer.toDouble();
		int digits = (int) log10(val) + 1;
		if (digits > 5)
			lcd->setNumDigits(digits);
		else
			lcd->setNumDigits(5);
		lcd->display(val);
		if (lowerLimitActive && val < lowerLimit)
		{
			setDigitColor(alarmDigitColor);	
#if 0
			/* TODO: Sent notification to event logger */
			timerOff();
			if (KMessageBox::questionYesNo(
				this, QString(i18n("%1\nLower limit exceeded!")).arg(title),
							  i18n("Alarm"),
							  i18n("Acknowledge"),
							  i18n("Disable Alarm")) == KMessageBox::No)
			{
				lowerLimitActive = FALSE;
			}
			timerOn();
#endif
		}
		else if (upperLimitActive && val > upperLimit)
		{
			setDigitColor(alarmDigitColor);
#if 0
			/* TODO: Sent notification to event logger */
			timerOff();
			if (KMessageBox::questionYesNo(
				this, QString(i18n("%1\nUpper limit exceeded!")).arg(title),
							  i18n("Alarm"),
							  i18n("Acknowledge"),
							  i18n("Disable Alarm")) == KMessageBox::No)
			{
				upperLimitActive = FALSE;
			}
			timerOn();
#endif
		}
		else
			setDigitColor(normalDigitColor);
	}
}

void
MultiMeter::resizeEvent(QResizeEvent*)
{
	frame->setGeometry(0, 0, width(), height());
}

void
MultiMeter::setTitle(const QString& t, const QString& u)
{
	title = t;
	unit = u;
	QString titleWithUnit = title;

	if (!unit.isEmpty() && showUnit)
		titleWithUnit = title + " [" + unit + "]";

	/* Changing the frame title may increase the width of the frame and
	 * hence breaks the layout. To avoid this, we save the original size
	 * and restore it after we have set the frame title. */
	QSize s = frame->size();
	frame->setTitle(titleWithUnit);
	frame->setGeometry(0, 0, s.width(), s.height());
}

bool
MultiMeter::createFromDOM(QDomElement& el)
{
	title = el.attribute("title");
	showUnit = el.attribute("showUnit").toInt();
	setTitle(title, unit);
	lowerLimitActive = el.attribute("lowerLimitActive").toInt();
	lowerLimit = el.attribute("lowerLimit").toLong();
	upperLimitActive = el.attribute("upperLimitActive").toInt();
	upperLimit = el.attribute("upperLimit").toLong();

	normalDigitColor = restoreColorFromDOM(el, "normalDigitColor", Qt::green);
	alarmDigitColor = restoreColorFromDOM(el, "alarmDigitColor", Qt::red);
	setBackgroundColor(restoreColorFromDOM(el, "backgroundColor", Qt::black));

	addSensor(el.attribute("hostName"), el.attribute("sensorName"), "");

	setModified(false);

	return (TRUE);
}

bool
MultiMeter::addToDOM(QDomDocument&, QDomElement& display, bool save)
{
	display.setAttribute("hostName", sensors.at(0)->hostName);
	display.setAttribute("sensorName", sensors.at(0)->name);
	display.setAttribute("title", title);
	display.setAttribute("showUnit", (int) showUnit);
	display.setAttribute("lowerLimitActive", (int) lowerLimitActive);
	display.setAttribute("lowerLimit", (int) lowerLimit);
	display.setAttribute("upperLimitActive", (int) upperLimitActive);
	display.setAttribute("upperLimit", (int) upperLimit);

	addColorToDOM(display, "normalDigitColor", normalDigitColor);
	addColorToDOM(display, "alarmDigitColor", alarmDigitColor);
	addColorToDOM(display, "backgroundColor", lcd->backgroundColor());

	if (save)
		setModified(FALSE);

	return (TRUE);
}

void
MultiMeter::settings()
{
	mms = new MultiMeterSettings(this, "MultiMeterSettings", TRUE);
	CHECK_PTR(mms);
	mms->title->setText(title);
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
	setTitle(mms->title->text(), unit);
	lowerLimitActive = mms->lowerLimitActive->isChecked();
	lowerLimit = mms->lowerLimit->text().toDouble();
	upperLimitActive = mms->upperLimitActive->isChecked();
	upperLimit = mms->upperLimit->text().toDouble();

	normalDigitColor = mms->normalDigitColor->getColor();
	alarmDigitColor = mms->alarmDigitColor->getColor();
	setBackgroundColor(mms->backgroundColor->getColor());

	repaint();
	setModified(TRUE);
}

void
MultiMeter::sensorError(int, bool err)
{
	if (err == sensors.at(0)->ok)
	{
		// this happens only when the sensorOk status needs to be changed.
		sensors.at(0)->ok = !err;
	}
	if (err)
		errorLabel->show();
	else
		errorLabel->hide();
}

void
MultiMeter::setDigitColor(const QColor& col)
{
	QPalette p;
	p.setColor(QColorGroup::Foreground, col);
	lcd->setPalette(p);
}

void
MultiMeter::setBackgroundColor(const QColor& col)
{
	lcd->setBackgroundColor(col);
	errorLabel->setBackgroundColor(col);
}
