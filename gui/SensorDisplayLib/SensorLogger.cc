/*
    KSysGuard, the KDE System Guard

	Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

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


#include <QDate>
#include <QDomNodeList>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QTextStream>

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>

#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include "SensorLogger.moc"
#include "SensorLoggerSettings.h"
#include "SensorLogger.h"

SLListViewItem::SLListViewItem(Q3ListView *parent)
	: Q3ListViewItem(parent)
{
}

LogSensor::LogSensor(Q3ListView *parent)
	: timerID( NONE ), lowerLimitActive( 0 ), upperLimitActive( 0 ),
	  lowerLimit( 0 ), upperLimit( 0 )
{
	Q_CHECK_PTR(parent);

	monitor = parent;

	lvi = new SLListViewItem(monitor);
	Q_CHECK_PTR(lvi);

	pixmap_running = UserIcon( "running" );
	pixmap_waiting = UserIcon( "waiting" );

	lvi->setPixmap(0, pixmap_waiting);
	lvi->setTextColor(monitor->palette().color( QPalette::Text ) );

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
	lvi->setTextColor(monitor->palette().color( QPalette::Text ) );
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
	QFile mLogFile(fileName);

	if (!mLogFile.open(QIODevice::ReadWrite | QIODevice::Append))
	{
		stopLogging();
		return;
	}

	switch (id)
	{
		case 42: {
			QTextStream stream(&mLogFile);
			double value = answer.toDouble();

			if (lowerLimitActive && value < lowerLimit)
			{
				timerOff();
				lowerLimitActive = false;
				lvi->setTextColor(monitor->palette().color( QPalette::Foreground ) );
				lvi->repaint();
				KNotifyClient::event(monitor->winId(), "sensor_alarm", QString("sensor '%1' at '%2' reached lower limit").arg(sensorName).arg(hostName));
				timerOn();
			} else if (upperLimitActive && value > upperLimit)
			{
				timerOff();
				upperLimitActive = false;
				lvi->setTextColor(monitor->palette().color( QPalette::Foreground ) );
				lvi->repaint();
				KNotifyClient::event(monitor->winId(), "sensor_alarm", QString("sensor '%1' at '%2' reached upper limit").arg(sensorName).arg(hostName));
				timerOn();
			}
			QDate date = QDateTime::currentDateTime().date();
			QTime time = QDateTime::currentDateTime().time();

			stream << QString("%1 %2 %3 %4 %5: %6\n").arg(date.shortMonthName(date.month())).arg(date.day()).arg(time.toString()).arg(hostName).arg(sensorName).arg(value);
		}
	}

	mLogFile.close();
}

SensorLogger::SensorLogger(QWidget *parent, const QString& title, bool isApplet)
	: KSGRD::SensorDisplay(parent, title, isApplet)
{
	monitor = new Q3ListView(this, "monitor");
	Q_CHECK_PTR(monitor);

	monitor->addColumn(i18n("Logging"));
	monitor->addColumn(i18n("Timer Interval"));
	monitor->addColumn(i18n("Sensor Name"));
	monitor->addColumn(i18n("Host Name"));
	monitor->addColumn(i18n("Log File"));

	QPalette cgroup = monitor->palette();
	cgroup.setColor(QPalette::Text, KSGRD::Style->firstForegroundColor());
	cgroup.setColor(QPalette::Base, KSGRD::Style->backgroundColor());
	cgroup.setColor(QPalette::Foreground, KSGRD::Style->alarmColor());
	monitor->setPalette( cgroup );
	monitor->setSelectionMode(Q3ListView::NoSelection);

	connect(monitor, SIGNAL(rightButtonClicked(Q3ListViewItem*, const QPoint&, int)), this, SLOT(RMBClicked(Q3ListViewItem*, const QPoint&, int)));

	setTitle(i18n("Sensor Logger"));

	logSensors.setAutoDelete(true);

	setPlotterWidget(monitor);

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

	sld = new SensorLoggerDlg(this, "SensorLoggerDlg");
	Q_CHECK_PTR(sld);

	if (sld->exec()) {
		if (!sld->fileName().isEmpty()) {
			LogSensor *sensor = new LogSensor(monitor);
			Q_CHECK_PTR(sensor);

			sensor->setHostName(hostName);
			sensor->setSensorName(sensorName);
			sensor->setFileName(sld->fileName());
			sensor->setTimerInterval(sld->timerInterval());
			sensor->setLowerLimitActive(sld->lowerLimitActive());
			sensor->setUpperLimitActive(sld->upperLimitActive());
			sensor->setLowerLimit(sld->lowerLimit());
			sensor->setUpperLimit(sld->upperLimit());

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
	sld = new SensorLoggerDlg(this, "SensorLoggerDlg");
	Q_CHECK_PTR(sld);

	sld->setFileName(sensor->getFileName());
	sld->setTimerInterval(sensor->getTimerInterval());
	sld->setLowerLimitActive(sensor->getLowerLimitActive());
	sld->setLowerLimit(sensor->getLowerLimit());
	sld->setUpperLimitActive(sensor->getUpperLimitActive());
	sld->setUpperLimit(sensor->getUpperLimit());

	if (sld->exec()) {
		if (!sld->fileName().isEmpty()) {
			sensor->setFileName(sld->fileName());
			sensor->setTimerInterval(sld->timerInterval());
			sensor->setLowerLimitActive(sld->lowerLimitActive());
			sensor->setUpperLimitActive(sld->upperLimitActive());
			sensor->setLowerLimit(sld->lowerLimit());
			sensor->setUpperLimit(sld->upperLimit());

			setModified(true);
		}
	}

	delete sld;
	sld = 0;

	return (true);
}

void
SensorLogger::configureSettings()
{
	QPalette cgroup = monitor->palette();

	sls = new SensorLoggerSettings(this, "SensorLoggerSettings");
	Q_CHECK_PTR(sls);

	connect( sls, SIGNAL( applyClicked() ), SLOT( applySettings() ) );

	sls->setTitle(title());
	sls->setForegroundColor(cgroup.text());
	sls->setBackgroundColor(cgroup.base());
	sls->setAlarmColor( cgroup.color( QPalette::Foreground ) );

	if (sls->exec())
		applySettings();

	delete sls;
	sls = 0;
}

void
SensorLogger::applySettings()
{
	QPalette cgroup = monitor->palette();

	setTitle(sls->title());

	cgroup.setColor(QPalette::Text, sls->foregroundColor());
	cgroup.setColor(QPalette::Base, sls->backgroundColor());
	cgroup.setColor(QPalette::Foreground, sls->alarmColor());
	monitor->setPalette( cgroup );

	setModified(true);
}

void
SensorLogger::applyStyle(void)
{
	QPalette cgroup = monitor->palette();

	cgroup.setColor(QPalette::Text, KSGRD::Style->firstForegroundColor());
	cgroup.setColor(QPalette::Base, KSGRD::Style->backgroundColor());
	cgroup.setColor(QPalette::Foreground, KSGRD::Style->alarmColor());
	monitor->setPalette( cgroup );

	setModified(true);
}

bool
SensorLogger::restoreSettings(QDomElement& element)
{
	QPalette cgroup = monitor->palette();

	cgroup.setColor(QPalette::Text, restoreColor(element, "textColor", Qt::green));
	cgroup.setColor(QPalette::Base, restoreColor(element, "backgroundColor", Qt::black));
	cgroup.setColor(QPalette::Foreground, restoreColor(element, "alarmColor", Qt::red));
	monitor->setPalette( cgroup );

	logSensors.clear();

	QDomNodeList dnList = element.elementsByTagName("logsensors");
	for (int i = 0; i < dnList.count(); i++) {
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

	SensorDisplay::restoreSettings(element);

	setModified(false);

	return (true);
}

bool
SensorLogger::saveSettings(QDomDocument& doc, QDomElement& element, bool save)
{
        saveColor(element, "textColor", monitor->palette().color( QPalette::Text ) );
	saveColor(element, "backgroundColor", monitor->palette().color( QPalette::Base ) );
	saveColor(element, "alarmColor", monitor->palette().color( QPalette::Foreground) );

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

	SensorDisplay::saveSettings(doc, element);

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
	monitor->setGeometry(10, 20, this->width() - 20, this->height() - 30);
}

LogSensor*
SensorLogger::getLogSensor(Q3ListViewItem* item)
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
SensorLogger::RMBClicked(Q3ListViewItem* item, const QPoint& point, int)
{
	Q3PopupMenu pm;
	if (hasSettingsDialog())
		pm.insertItem(i18n("&Properties"), 1);
	pm.insertItem(i18n("&Remove Display"), 2);
	pm.insertSeparator(-1);
	pm.insertItem(i18n("&Remove Sensor"), 3);
	pm.insertItem(i18n("&Edit Sensor..."), 4);

	if ( !item )
	{
		pm.setItemEnabled( 3, false );
		pm.setItemEnabled( 4, false );
	}
	else
	{
		LogSensor* sensor = getLogSensor(item);

		if ( sensor && sensor->isLogging() )
			pm.insertItem(i18n("St&op Logging"), 6);
		else
			pm.insertItem(i18n("S&tart Logging"), 5);
	}

	switch (pm.exec(point))
	{
	case 1:
		configureSettings();
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
			editSensor(sensor);
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
