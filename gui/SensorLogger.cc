/*
    KSysGuard, the KDE System Guard

	Copyright (c) 2001 Tobias Koenig <tokoe82@yahoo.de>

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

	$Id$
*/

#include <qcheckbox.h>
#include <qpushbutton.h>

#include <kapplication.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knumvalidator.h>

#include <ksgrd/ColorPicker.h>
#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include "SensorLogger.moc"
#include "SensorLoggerSettings.h"

SLListViewItem::SLListViewItem(QListView *parent)
	: QListViewItem(parent)
{
}

LogSensor::LogSensor(QListView *parent)
{
	Q_CHECK_PTR(parent);

	monitor = parent;

	lvi = new SLListViewItem(monitor);
	Q_CHECK_PTR(lvi);

	lowerLimit = 0;
	lowerLimitActive = 0;
	upperLimit = 0;
	upperLimitActive = 0;

	KIconLoader *icons = new KIconLoader();
	Q_CHECK_PTR(icons);
	pixmap_running = icons->loadIcon("running", KIcon::Small, KIcon::SizeSmall);
	pixmap_waiting = icons->loadIcon("waiting", KIcon::Small, KIcon::SizeSmall);
	delete icons;

	lvi->setPixmap(0, pixmap_waiting);
	lvi->setTextColor(monitor->colorGroup().text());

	monitor->insertItem(lvi);
}

LogSensor::~LogSensor(void)
{
	if ((lvi) && (monitor))
		delete lvi;
}

void
LogSensor::startLogging(void)
{
	lvi->setPixmap(0, pixmap_running);
	timerOn();
}

void
LogSensor::stopLogging(void)
{
	lvi->setPixmap(0, pixmap_waiting);
	lvi->setTextColor(monitor->colorGroup().text());
	lvi->repaint();
	timerOff();
}

void
LogSensor::timerEvent(QTimerEvent*)
{
	KSGRD::SensorMgr->sendRequest(hostName, sensorName, (KSGRD::SensorClient*) this, 42);
}

void
LogSensor::answerReceived(int id, const QString& answer)
{
	logFile = new QFile(fileName);
	Q_CHECK_PTR(logFile);

	if (!logFile->open(IO_ReadWrite | IO_Append))
	{
		stopLogging();
		delete logFile;
		return;
	}

	switch (id)
	{
		case 42: {
			QTextStream stream(logFile);
			double value = answer.toDouble();

			if (lowerLimitActive && value < lowerLimit)
			{
				timerOff();
				lowerLimitActive = false;
				lvi->setTextColor(monitor->colorGroup().foreground());
				lvi->repaint();
				KNotifyClient::event("sensor_alarm", QString("sensor '%1' at '%2' reached lower limit").arg(sensorName).arg(hostName));
				timerOn();
			} else if (upperLimitActive && value > upperLimit)
			{
				timerOff();
				upperLimitActive = false;
				lvi->setTextColor(monitor->colorGroup().foreground());
				lvi->repaint();
				KNotifyClient::event("sensor_alarm", QString("sensor '%1' at '%2' reached upper limit").arg(sensorName).arg(hostName));
				timerOn();
			}
			QDate date = QDateTime::currentDateTime().date();
			QTime time = QDateTime::currentDateTime().time();

			stream << QString("%1 %2 %3 %4 %5: %6\n").arg(date.shortMonthName(date.month())).arg(date.day()).arg(time.toString()).arg(hostName).arg(sensorName).arg(value);
		}
	}

	logFile->close();
	delete logFile;
}

SensorLogger::SensorLogger(QWidget *parent, const char *name, const QString& title)
	: KSGRD::SensorDisplay(parent, name, title)
{
	monitor = new QListView(this, "monitor");
	Q_CHECK_PTR(monitor);

	monitor->addColumn(i18n("Logging"));
	monitor->addColumn(i18n("TimerInterval"));
	monitor->addColumn(i18n("SensorName"));
	monitor->addColumn(i18n("HostName"));
	monitor->addColumn(i18n("LogFile"));

	QColorGroup cgroup = monitor->colorGroup();
	cgroup.setColor(QColorGroup::Text, KSGRD::Style->getFgColor1());
	cgroup.setColor(QColorGroup::Base, KSGRD::Style->getBackgroundColor());
	cgroup.setColor(QColorGroup::Foreground, KSGRD::Style->getAlarmColor());
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));
	monitor->setSelectionMode(QListView::NoSelection);

	connect(monitor, SIGNAL(rightButtonClicked(QListViewItem*, const QPoint&, int)), this, SLOT(RMBClicked(QListViewItem*, const QPoint&, int)));
	
	frame->setTitle(i18n("Sensor Logger"));

	logSensors.setAutoDelete(true);

	setMinimumSize(50, 25);
	setModified(false);
}

SensorLogger::~SensorLogger(void)
{
}

bool
SensorLogger::addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString&)
{
	if (sensorType != "integer" && sensorType != "float")
		return (false);

	sld = new SensorLoggerDlg(this, "SensorLoggerDlg", true);
	Q_CHECK_PTR(sld);

	sld->applyButton->hide();
	connect(sld->fileButton, SIGNAL(clicked()), this, SLOT(fileSelect()));

	if (sld->exec()) {
		if (!sld->fileName->text().isEmpty()) {
			LogSensor *sensor = new LogSensor(monitor);
			Q_CHECK_PTR(sensor);

			sensor->setHostName(hostName);
			sensor->setSensorName(sensorName);
			sensor->setFileName(sld->fileName->text());
			sensor->setTimerInterval(sld->timer->text().toInt());
			sensor->setLowerLimitActive(sld->lowerLimitActive->isChecked());
			sensor->setUpperLimitActive(sld->upperLimitActive->isChecked());
			sensor->setLowerLimit(sld->lowerLimit->text().toDouble());
			sensor->setUpperLimit(sld->upperLimit->text().toDouble());

			logSensors.append(sensor);

			setModified(true);
		}
	}

	delete sld;
	sld = 0;

	return (true);
}

bool
SensorLogger::editSensor(LogSensor* sensor)
{
	sld = new SensorLoggerDlg(this, "SensorLoggerDlg", true);
	Q_CHECK_PTR(sld);

	connect(sld->fileButton, SIGNAL(clicked()), this, SLOT(fileSelect()));

	sld->fileName->setText(sensor->getFileName());
	sld->timer->setValue(sensor->getTimerInterval());
	sld->lowerLimitActive->setChecked(sensor->getLowerLimitActive());
	sld->lowerLimit->setText(QString("%1").arg(sensor->getLowerLimit()));
	sld->lowerLimit->setValidator(new KFloatValidator(sld->lowerLimit));
	sld->upperLimitActive->setChecked(sensor->getUpperLimitActive());
	sld->upperLimit->setText(QString("%1").arg(sensor->getUpperLimit()));
	sld->upperLimit->setValidator(new KFloatValidator(sld->upperLimit));


	if (sld->exec()) {
		if (!sld->fileName->text().isEmpty()) {
			sensor->stopLogging();
			sensor->setFileName(sld->fileName->text());
			sensor->setTimerInterval(sld->timer->text().toInt());
			sensor->setLowerLimitActive(sld->lowerLimitActive->isChecked());
			sensor->setUpperLimitActive(sld->upperLimitActive->isChecked());
			sensor->setLowerLimit(sld->lowerLimit->text().toDouble());
			sensor->setUpperLimit(sld->upperLimit->text().toDouble());

			setModified(true);
		}
	}

	delete sld;
	sld = 0;

	return (true);
}

void
SensorLogger::fileSelect(void)
{
	QString fileName = KFileDialog::getSaveFileName();
	if (!fileName.isEmpty())
		sld->fileName->setText(fileName);
}


void
SensorLogger::settings()
{
	QColorGroup cgroup = monitor->colorGroup();

	sls = new SensorLoggerSettings(this, "SensorLoggerSettings", true);
	Q_CHECK_PTR(sls);

	connect(sls->applyButton, SIGNAL(clicked()), this, SLOT(applySettings()));

	sls->foregroundColor->setColor(cgroup.text());
	sls->backgroundColor->setColor(cgroup.base());
	sls->alarmColor->setColor(cgroup.foreground());
	sls->title->setText(title());

	if (sls->exec())
		applySettings();

	delete sls;
	sls = 0;
}

void
SensorLogger::applySettings()
{
	QColorGroup cgroup = monitor->colorGroup();

	cgroup.setColor(QColorGroup::Text, sls->foregroundColor->getColor());
	cgroup.setColor(QColorGroup::Base, sls->backgroundColor->getColor());
	cgroup.setColor(QColorGroup::Foreground, sls->alarmColor->getColor());
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));

	title(sls->title->text());

	setModified(true);
}

void
SensorLogger::applyStyle(void)
{
	QColorGroup cgroup = monitor->colorGroup();

	cgroup.setColor(QColorGroup::Text, KSGRD::Style->getFgColor1());
	cgroup.setColor(QColorGroup::Base, KSGRD::Style->getBackgroundColor());
	cgroup.setColor(QColorGroup::Foreground, KSGRD::Style->getAlarmColor());
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));

	setModified(true);
}

bool
SensorLogger::createFromDOM(QDomElement& element)
{
	QColorGroup cgroup = monitor->colorGroup();

	cgroup.setColor(QColorGroup::Text, restoreColorFromDOM(element, "textColor", Qt::green));
	cgroup.setColor(QColorGroup::Base, restoreColorFromDOM(element, "backgroundColor", Qt::black));
	cgroup.setColor(QColorGroup::Foreground, restoreColorFromDOM(element, "alarmColor", Qt::red));
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));

	logSensors.clear();

	QDomNodeList dnList = element.elementsByTagName("logsensors");
	for (uint i = 0; i < dnList.count(); i++) {
		QDomElement element = dnList.item(i).toElement();
		LogSensor* sensor = new LogSensor(monitor);
		Q_CHECK_PTR(sensor);

		sensor->setHostName(element.attribute("hostName"));
		sensor->setSensorName(element.attribute("sensorName"));
		sensor->setFileName(element.attribute("fileName"));
		sensor->setTimerInterval(element.attribute("timerInterval").toInt());
		sensor->setLowerLimitActive(element.attribute("lowerLimitActive").toInt());
		sensor->setLowerLimit(element.attribute("lowerLimit").toDouble());
		sensor->setUpperLimitActive(element.attribute("upperLimitActive").toInt());
		sensor->setUpperLimit(element.attribute("upperLimit").toDouble());

		logSensors.append(sensor);
	}

	internCreateFromDOM(element);

	setModified(false);

	return (true);
}

bool
SensorLogger::addToDOM(QDomDocument& doc, QDomElement& element, bool save)
{
	addColorToDOM(element, "textColor", monitor->colorGroup().text());
	addColorToDOM(element, "backgroundColor", monitor->colorGroup().base());
	addColorToDOM(element, "alarmColor", monitor->colorGroup().foreground());

	for (LogSensor* sensor = logSensors.first(); sensor != 0; sensor = logSensors.next())
	{
		QDomElement log = doc.createElement("logsensors");
		log.setAttribute("sensorName", sensor->getSensorName());
		log.setAttribute("hostName", sensor->getHostName());
		log.setAttribute("fileName", sensor->getFileName());
		log.setAttribute("timerInterval", sensor->getTimerInterval());
		log.setAttribute("lowerLimitActive", QString("%1").arg(sensor->getLowerLimitActive()));
		log.setAttribute("lowerLimit", QString("%1").arg(sensor->getLowerLimit()));
		log.setAttribute("upperLimitActive", QString("%1").arg(sensor->getUpperLimitActive()));
		log.setAttribute("upperLimit", QString("%1").arg(sensor->getUpperLimit()));
		
		element.appendChild(log);
	}

	internAddToDOM(doc, element);

	if (save)
		setModified(false);

	return (true);
}

void
SensorLogger::answerReceived(int, const QString&)
{
 // we do not use this, since all answers are received by the LogSensors
}

void
SensorLogger::resizeEvent(QResizeEvent*)
{
	frame->setGeometry(0, 0, this->width(), this->height());
	monitor->setGeometry(10, 20, this->width() - 20, this->height() - 30);
}

LogSensor*
SensorLogger::getLogSensor(QListViewItem* item)
{
	for (LogSensor* sensor = logSensors.first(); sensor != 0; sensor = logSensors.next())
	{
		if (item == sensor->getListViewItem()) {
			return sensor;
		}
	}

	return NULL;
}

void
SensorLogger::RMBClicked(QListViewItem* item, const QPoint& point, int)
{
	QPopupMenu pm;
	if (hasSettingsDialog())
		pm.insertItem(i18n("&Properties"), 1);
	pm.insertItem(i18n("&Remove Display"), 2);
	pm.insertSeparator(-1);
	pm.insertItem(i18n("&Remove Sensor"), 3);
	pm.insertItem(i18n("&Edit Sensor"), 4);
	pm.insertItem(i18n("S&tart Logging"), 5);
	pm.insertItem(i18n("St&op Logging"), 6);

	switch (pm.exec(point))
	{
	case 1:
		this->settings();
		break;
	case 2: {
		QCustomEvent* ev = new QCustomEvent(QEvent::User);
		ev->setData(this);
		kapp->postEvent(parent(), ev);
		break;
		}
	case 3:	{
		LogSensor* sensor = getLogSensor(item);
		if (sensor)
			logSensors.remove(sensor);
		break;
		}
	case 4: {
		LogSensor* sensor = getLogSensor(item);
		if (sensor)
			this->editSensor(sensor);
		break;
		}
	case 5: {
		LogSensor* sensor = getLogSensor(item);
		if (sensor)
			sensor->startLogging();
		break;
		}
	case 6: {
		LogSensor* sensor = getLogSensor(item);
		if (sensor)
			sensor->stopLogging();
		break;
		}
	}
}
