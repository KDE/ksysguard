/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

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

#include <klocale.h>
#include <kmessagebox.h>

#include "MultiMeterSettings.h"
#include "SensorManager.h"
#include "MultiMeter.moc"

MultiMeter::MultiMeter(QWidget* parent, const char* name,
					   const QString& t, int, int)
	: SensorDisplay(parent, name)
{
	frame = new QGroupBox(1, Qt::Vertical, title, this, "meterFrame"); 
	CHECK_PTR(frame);

	showUnit = TRUE;
	lowerLimit = upperLimit = 0;
	lowerLimitActive = upperLimitActive = FALSE;

	setTitle(t, unit);

	lcd = new QLCDNumber(frame, "meterLCD");
	CHECK_PTR(lcd);
	lcd->setSegmentStyle(QLCDNumber::Filled);
	lcd->setBackgroundColor(Qt::black);
	QPalette p;
	p.setColor(QColorGroup::Foreground, Qt::green);
	lcd->setPalette(p);
	lcd->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
								   QSizePolicy::Expanding, FALSE));

	/* All RMB clicks to the lcd widget will be handled by 
	 * SensorDisplay::eventFilter. */
	lcd->installEventFilter(this);

	setMinimumSize(50, 25);
	modified = FALSE;
}

bool
MultiMeter::addSensor(const QString& hostName, const QString& sensorName,
					  const QString& title)
{
	registerSensor(hostName, sensorName, title);

	/* To differentiate between answers from value requests and info
	 * requests we use 100 for info requests. */
	sendRequest(hostName, sensorName + "?", 100);

	modified = TRUE;
	return (TRUE);
}

void
MultiMeter::answerReceived(int id, const QString& answer)
{
	if (id == 100)
	{
		SensorIntegerInfo info(answer);
		setTitle(title, SensorMgr->translateUnit(info.getUnit()));
		timerOn();
	}
	else
	{
		long val = answer.toInt();
		int digits = (int) log10((double) val) + 1;
		if (digits > 5)
			lcd->setNumDigits(digits);
		else
			lcd->setNumDigits(5);
		lcd->display((int) val);
		if (lowerLimitActive && val < lowerLimit)
		{
			timerOff();
			QColor oldColor = lcd->backgroundColor();
			lcd->setBackgroundColor(QColor(255, 0, 0));
			if (KMessageBox::questionYesNo(
				this, QString(i18n("%1\nLower limit exceeded!")).arg(title),
							  i18n("Alarm"),
							  i18n("Acknowledge"),
							  i18n("Disable Alarm")) == KMessageBox::No)
			{
				lowerLimitActive = FALSE;
			}
			lcd->setBackgroundColor(oldColor);
			timerOn();
		}
		if (upperLimitActive && val > upperLimit)
		{
			timerOff();
			QColor oldColor = lcd->backgroundColor();
			lcd->setBackgroundColor(QColor(255, 0, 0));
			if (KMessageBox::questionYesNo(
				this, QString(i18n("%1\nUpper limit exceeded!")).arg(title),
							  i18n("Alarm"),
							  i18n("Acknowledge"),
							  i18n("Disable Alarm")) == KMessageBox::No)
			{
				upperLimitActive = FALSE;
			}
			lcd->setBackgroundColor(oldColor);
			timerOn();
		}
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

	frame->setTitle(titleWithUnit);
}

bool
MultiMeter::load(QDomElement& el)
{
	modified = false;

	title = el.attribute("title");
	showUnit = el.attribute("showUnit").toInt();
	setTitle(title, unit);
	lowerLimitActive = el.attribute("lowerLimitActive").toInt();
	lowerLimit = el.attribute("lowerLimit").toLong();
	upperLimitActive = el.attribute("upperLimitActive").toInt();
	upperLimit = el.attribute("upperLimit").toLong();
	addSensor(el.attribute("hostName"), el.attribute("sensorName"), "");

	return (TRUE);
}

bool
MultiMeter::save(QDomDocument&, QDomElement& display)
{
	display.setAttribute("hostName", *hostNames.at(0));
	display.setAttribute("sensorName", *sensorNames.at(0));
	display.setAttribute("title", title);
	display.setAttribute("showUnit", (int) showUnit);
	display.setAttribute("lowerLimitActive", (int) lowerLimitActive);
	display.setAttribute("lowerLimit", (int) lowerLimit);
	display.setAttribute("upperLimitActive", (int) upperLimitActive);
	display.setAttribute("upperLimit", (int) upperLimit);
	modified = FALSE;

	return (TRUE);
}

void
MultiMeter::settings()
{
	mms = new MultiMeterSettings(this, "MultiMeterSettings", TRUE);
	CHECK_PTR(mms);
	mms->title->setText(title);
	mms->showUnit->setChecked(showUnit);
	mms->lowerLimitActive->setChecked(lowerLimitActive);
	mms->lowerLimit->setValue(lowerLimit);
	mms->upperLimitActive->setChecked(upperLimitActive);
	mms->upperLimit->setValue(upperLimit);
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
	lowerLimit = mms->lowerLimit->text().toLong();
	upperLimitActive = mms->upperLimitActive->isChecked();
	upperLimit = mms->upperLimit->text().toLong();

	modified = TRUE;
}

void
MultiMeter::sensorError(bool err)
{
	if (err == sensorOk)
	{
		// this happens only when the sensorOk status needs to be changed.
		frame->setEnabled(!err);
		sensorOk = !err;
	}
}
