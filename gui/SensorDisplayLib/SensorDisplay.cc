/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>
    
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

#include <qcheckbox.h>
#include <qdom.h>
#include <qpopupmenu.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qbitmap.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "SensorDisplay.h"
#include "SensorDisplay.moc"
#include "SensorManager.h"
#include "TimerSettings.h"


using namespace KSGRD;

SensorDisplay::SensorDisplay(QWidget* parent, const char* name, const QString& title) :
	QWidget(parent, name)
{
	sensors.setAutoDelete(true);

	// default interval is 2 seconds.
	timerInterval = 2000;
	globalUpdateInterval = true;
	modified = false;
	showUnit = false;
	timerId = NONE;
	frame = 0;
	errorIndicator = 0;
	plotterWdg = 0;

	pauseOnHide = false;
	pausedWhileHidden = false;

	timerOn();
	QWhatsThis::add(this, "dummy");

	frame = new QGroupBox(2, Qt::Vertical, "", this, "displayFrame"); 
	Q_CHECK_PTR(frame);
	setTitle(title);

	setMinimumSize(16, 16);
	setModified(false);
	setSensorOk(false);

	/* All RMB clicks to the box frame will be handled by 
	 * SensorDisplay::eventFilter. */
	frame->installEventFilter(this);

	/* Let's call updateWhatsThis() in case the derived class does not do
	 * this. */
	updateWhatsThis();
	setFocusPolicy(QWidget::StrongFocus);
}

SensorDisplay::~SensorDisplay()
{
	if (SensorMgr != 0)
		SensorMgr->unlinkClient(this);
	killTimer(timerId);
}

void
SensorDisplay::registerSensor(SensorProperties* sp)
{
	/* Make sure that we have a connection established to the specified
	 * host. When a work sheet has been saved while it had dangling
	 * sensors, the connect info is not saved in the work sheet. In such
	 * a case the user can re-enter the connect information and the
	 * connection will be established. */
	if (!SensorMgr->engageHost(sp->hostName))
	{
		QString msg = i18n("Impossible to connect to \'%1\'!")
			.arg(sp->hostName);
		KMessageBox::error(this, msg);
	}

	sensors.append(sp);
}

void
SensorDisplay::unregisterSensor(uint idx)
{
	sensors.remove(idx);
}

void
SensorDisplay::setupTimer()
{
	ts = new TimerSettings(this, "TimerSettings", true);
	Q_CHECK_PTR(ts);

	connect(ts->useGlobalUpdate, SIGNAL(toggled(bool)), this, 
			SLOT(timerToggled(bool)));

	ts->useGlobalUpdate->setChecked(globalUpdateInterval);
	ts->interval->setValue(timerInterval / 1000);

    timerToggled( globalUpdateInterval );

	if (ts->exec()) {
		if (ts->useGlobalUpdate->isChecked()) {
			globalUpdateInterval = true;

			SensorBoard* sb = dynamic_cast<SensorBoard*>(parentWidget());
			if (!sb) {
				kdDebug() << "dynamic cast lacks" << endl;
				setUpdateInterval(2);
			} else {
				setUpdateInterval(sb->updateInterval());
			}
		} else {
			globalUpdateInterval = false;
			setUpdateInterval(ts->interval->text().toInt());
		}
		setModified(true);
	}

	delete ts;
}

void
SensorDisplay::timerToggled(bool value)
{
	ts->interval->setEnabled(!value);
	ts->TextLabel1->setEnabled(!value);
}

void
SensorDisplay::timerEvent(QTimerEvent*)
{
	int i = 0;
	for (SensorProperties* s = sensors.first(); s; s = sensors.next(), ++i)
		sendRequest(s->hostName, s->name, i);
}

void
SensorDisplay::resizeEvent(QResizeEvent*)
{
	frame->setGeometry(rect());
}

bool
SensorDisplay::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QEvent::MouseButtonPress &&
		((QMouseEvent*) e)->button() == RightButton)
	{
		QPopupMenu pm;
		if (hasSettingsDialog())
			pm.insertItem(i18n("&Properties"), 1);
		pm.insertItem(i18n("&Remove Display"), 2);
		pm.insertSeparator();
		pm.insertItem(i18n("&Setup Update Interval"), 3);
		if (timerId == NONE)
			pm.insertItem(i18n("&Continue Update"), 4);
		else
			pm.insertItem(i18n("P&ause Update"), 5);
		switch (pm.exec(QCursor::pos()))
		{
		case 1:
			settings();
			break;
		case 2: {
			QCustomEvent* ev = new QCustomEvent(QEvent::User);
			ev->setData(this);
			kapp->postEvent(parent(), ev);
			}
			break;
		case 3:
			setupTimer();
			break;
		case 4:
			timerOn();
			setModified(true);
			break;
		case 5:
			timerOff();
			setModified(true);
			break;
		}
		return (true);
	}
	else if (e->type() == QEvent::MouseButtonRelease &&
			 ((QMouseEvent*) e)->button() == LeftButton)
	{
		setFocus();
	}
	return QWidget::eventFilter(o, e);
}

void
SensorDisplay::sendRequest(const QString& hostName, const QString& cmd,
						   int id)
{
	if (!SensorMgr->sendRequest(hostName, cmd,
								(SensorClient*) this, id))
		sensorError(id, true);
}

void
SensorDisplay::sensorError(int sensorId, bool err)
{
	if (sensorId >= (int) sensors.count() || sensorId < 0)
		return;

	if (err == sensors.at(sensorId)->ok)
	{
		// this happens only when the sensorOk status needs to be changed.
		sensors.at(sensorId)->ok = !err;
	}

	bool ok = true;
	for (uint i = 0; i < sensors.count(); ++i)
		if (!sensors.at(i)->ok)
		{
			ok = false;
			break;
		}

	setSensorOk(ok);
}

void
SensorDisplay::updateWhatsThis()
{
	QWhatsThis::add(this, QString(i18n(
		"<qt><p>This is a sensor display. To customize a sensor display click "
		"and hold the right mouse button on either the frame or the "
		"display box and select the <i>Properties</i> entry from the popup "
		"menu. Select <i>Remove</i> to delete the display from the work "
		"sheet.</p>%1</qt>")).arg(additionalWhatsThis()));
}

void
SensorDisplay::collectHosts(QValueList<QString>& list)
{
	for (SensorProperties* s = sensors.first(); s; s = sensors.next())
		if (!list.contains(s->hostName))
			list.append(s->hostName);
}

QColor
SensorDisplay::restoreColorFromDOM(QDomElement& de, const QString& attr,
								   const QColor& fallback)
{
	bool ok;
	uint c = de.attribute(attr).toUInt(&ok);
	if (!ok)
		return (fallback);
	else
		return (QColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF));
}

void
SensorDisplay::addColorToDOM(QDomElement& de, const QString& attr,
							 const QColor& col)
{
	int r, g, b;
	col.rgb(&r, &g, &b);
	de.setAttribute(attr, (r << 16) | (g << 8) | b);
}

void
SensorDisplay::internAddToDOM(QDomDocument& doc, QDomElement& element)
{
	element.setAttribute("title", getTitle());
	element.setAttribute("unit", getUnit());
	element.setAttribute("showUnit", showUnit);

	if (globalUpdateInterval)
		element.setAttribute("globalUpdate", "1");
	else
		element.setAttribute("updateInterval", timerInterval / 1000);

	if (timerId == NONE)
		element.setAttribute("pause", 1);
	else
		element.setAttribute("pause", 0);
}

void
SensorDisplay::internCreateFromDOM(QDomElement& element)
{
	showUnit = element.attribute("showUnit", "0").toInt();
	setUnit(element.attribute("unit", QString::null));
	setTitle(element.attribute("title", QString::null));

	if (element.attribute("updateInterval") != QString::null) {
		globalUpdateInterval = false;
		setUpdateInterval(element.attribute("updateInterval", "2").toInt());
	} else {
		globalUpdateInterval = true;

		SensorBoard* sb = dynamic_cast<SensorBoard*>(parentWidget());
		if (!sb) {
			kdDebug() << "dynamic cast lacks" << endl;
			setUpdateInterval(2);
		} else {
			setUpdateInterval(sb->updateInterval());
		}
	}

	if (element.attribute("pause", "0").toInt() == 0)
		timerOn();
	else
		timerOff();
}

bool
SensorDisplay::addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& description)
{
	registerSensor(new SensorProperties(hostName, sensorName, sensorType,
									   	description));
	return (true);
}

bool
SensorDisplay::removeSensor(uint idx)
{
	unregisterSensor(idx);
	return (true);
}

void
SensorDisplay::setUpdateInterval(uint interval)
{
	bool timerActive = timerId != NONE;

	if (timerActive)
		timerOff();
	timerInterval = interval * 1000;
	if (timerActive)
		timerOn();
}

QString
SensorDisplay::additionalWhatsThis()
{
	return QString::null;
}

void
SensorDisplay::sensorLost(int reqId)
{
	sensorError(reqId, true);
}

bool
SensorDisplay::createFromDOM(QDomElement&)
{
	// should never been used.
	return (false);
}

bool
SensorDisplay::addToDOM(QDomDocument&, QDomElement&, bool)
{
	// should never been used.
	return (false);
}

void
SensorDisplay::setIsOnTop(bool onTop)
{
	if (!pauseOnHide)
		return;
	
	if (onTop && pausedWhileHidden)
	{
		timerOn();
		pausedWhileHidden = false;
	}
	else if (!onTop && timerId != NONE)
	{
		timerOff();
		pausedWhileHidden = true;
	}
}

void
SensorDisplay::timerOff()
{
	if (timerId != NONE)
	{
		killTimer(timerId);
		timerId = NONE;
	} 
}

void
SensorDisplay::timerOn()
{
	if (timerId == NONE)
	{
		timerId = startTimer(timerInterval);
	}
}

void
SensorDisplay::rmbPressed()
{
	emit(showPopupMenu(this));
}

void
SensorDisplay::setModified(bool mfd)
{
	if (mfd != modified)
	{
		modified = mfd;
		emit displayModified(modified);
	}
}
		
void
SensorDisplay::focusInEvent(QFocusEvent*)
{
	frame->setLineWidth(2);
}

void
SensorDisplay::focusOutEvent(QFocusEvent*)
{
	frame->setLineWidth(1);
}

void
SensorDisplay::setSensorOk(bool ok)
{
	if (ok)
	{
		if (errorIndicator)
		{
			delete errorIndicator;
			errorIndicator = 0;
		}
	}
	else 
	{
		if (errorIndicator)
			return;

		KIconLoader iconLoader;
		QPixmap errorIcon = iconLoader.loadIcon("connect_creating", 
												KIcon::Desktop,
												KIcon::SizeSmall);
		if (plotterWdg == 0)
			return;

		errorIndicator = new QWidget(plotterWdg);
		errorIndicator->setErasePixmap(errorIcon);
		errorIndicator->resize(errorIcon.size());
		if (errorIcon.mask())
			errorIndicator->setMask(*errorIcon.mask());
		errorIndicator->move(0, 0);
		errorIndicator->show();
	}
}

void
SensorDisplay::setTitle(const QString& t)
{
	title = t;

	/* Changing the frame title may increase the width of the frame and
	 * hence breaks the layout. To avoid this, we save the original size
	 * and restore it after we have set the frame title. */
	QSize s = frame->size();

	if (showUnit && !unit.isEmpty())
		frame->setTitle(title + " [" + unit + "]");
	else
		frame->setTitle(title);

	frame->setGeometry(0, 0, s.width(), s.height());
}

QString
SensorDisplay::getTitle()
{
	return title;
}

void
SensorDisplay::setUnit(const QString& u)
{
	unit = u;
}

QString
SensorDisplay::getUnit()
{
	return unit;
}

void
SensorDisplay::registerPlotterWidget(QWidget* plotter)
{
	plotterWdg = plotter;
}
