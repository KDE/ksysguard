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
*/

#include <kapp.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include <qtextstream.h>

#include "ColorPicker.h"
#include "SensorManager.h"
#include "StyleEngine.h"

#include "SensorLogger.moc"
#include "SensorLoggerSettings.h"

#include <stdio.h>

LogSensor::LogSensor(QListView *parent)
{
	CHECK_PTR(parent);

	monitor = parent;
	
	lvi = new QListViewItem(monitor);
	CHECK_PTR(lvi);

	KIconLoader *icons = new KIconLoader();
	CHECK_PTR(icons);
	pixmap_running = icons->loadIcon("running", KIcon::Small, KIcon::SizeSmall);
	pixmap_waiting = icons->loadIcon("waiting", KIcon::Small, KIcon::SizeSmall);
	delete icons;

	lvi->setPixmap(0, pixmap_waiting);

	monitor->insertItem(lvi);
}

LogSensor::~LogSensor(void)
{
	if ((lvi) && (monitor))
		monitor->takeItem(lvi);
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
	timerOff();
}

void
LogSensor::timerEvent(QTimerEvent*)
{
	SensorMgr->sendRequest(hostName, sensorName, (SensorClient*) this, 42);
}

void
LogSensor::answerReceived(int id, const QString& answer)
{
	logFile = new QFile(fileName);
	CHECK_PTR(logFile);

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

			SensorTokenizer lines(answer, '\n');

			for (uint i = 0; i < lines.numberOfTokens(); i++) {
				stream << QString("%1\t%2\n").arg(QTime::currentTime().toString()).arg(lines[0]);
			}
		}
	}

	logFile->close();
	delete logFile;
}

SensorLogger::SensorLogger(QWidget *parent, const char *name, const QString&)
	: SensorDisplay(parent, name)
{
	monitor = new QListView(this, "monitor");
	CHECK_PTR(monitor);

	monitor->addColumn(i18n("Logging"));
	monitor->addColumn(i18n("TimerInterval"));
	monitor->addColumn(i18n("SensorName"));
	monitor->addColumn(i18n("HostName"));
	monitor->addColumn(i18n("LogFile"));

	connect(monitor, SIGNAL(rightButtonClicked(QListViewItem*, const QPoint&, int)), this, SLOT(RMBClicked(QListViewItem*, const QPoint&, int)));
	
	frame->setTitle(i18n("Sensor Logger"));

	logSensors.setAutoDelete(true);

	skip = 1;

	setMinimumSize(50, 25);
	setModified(false);
}

SensorLogger::~SensorLogger(void)
{
}

bool
SensorLogger::addSensor(const QString& hostName, const QString& sensorName, const QString&)
{
	if (skip) {
		skip = false;
		return (true);
	}

	SLDlg = new SensorLoggerDlg(this, "SensorLoggerDlg", true);
	CHECK_PTR(SLDlg);

	connect(SLDlg->fileButton, SIGNAL(clicked()), this, SLOT(fileSelect()));

	if (SLDlg->exec()) {
		if (!SLDlg->fileName->text().isEmpty()) {
			LogSensor *sensor = new LogSensor(monitor);
			CHECK_PTR(sensor);

			sensor->setHostName(hostName);
			sensor->setSensorName(sensorName);
			sensor->setFileName(SLDlg->fileName->text());
			sensor->setTimerInterval(SLDlg->timer->text().toInt());

			logSensors.append(sensor);

			setModified(true);
		}
	}

	delete SLDlg;

	return (true);
}

bool
SensorLogger::editSensor(LogSensor* sensor)
{
	SLDlg = new SensorLoggerDlg(this, "SensorLoggerDlg", true);
	CHECK_PTR(SLDlg);

	connect(SLDlg->fileButton, SIGNAL(clicked()), this, SLOT(fileSelect()));

	SLDlg->fileName->setText(sensor->getFileName());
	SLDlg->timer->setValue(sensor->getTimerInterval());

	if (SLDlg->exec()) {
		if (!SLDlg->fileName->text().isEmpty()) {
			sensor->stopLogging();
			sensor->setFileName(SLDlg->fileName->text());
			sensor->setTimerInterval(SLDlg->timer->text().toInt());

			setModified(true);
		}
	}

	delete SLDlg;

	return (true);
}

void
SensorLogger::fileSelect(void)
{
	QString fileName = KFileDialog::getSaveFileName();
	if (!fileName.isEmpty())
		SLDlg->fileName->setText(fileName);
}


void
SensorLogger::settings()
{
	QColorGroup cgroup = monitor->colorGroup();

	sls = new SensorLoggerSettings(this, "SensorLoggerSettings", TRUE);
	CHECK_PTR(sls);
	connect(sls->applyButton, SIGNAL(clicked()), this, SLOT(applySettings()));

	sls->foregroundColor->setColor(cgroup.text());
	sls->backgroundColor->setColor(cgroup.base());
	sls->title->setText(title);

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
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));

	title = sls->title->text();

	frame->setTitle(title);

	setModified(true);
}

void
SensorLogger::applyStyle(void)
{
	QColorGroup cgroup = monitor->colorGroup();

	cgroup.setColor(QColorGroup::Text, Style->getFgColor1());
	cgroup.setColor(QColorGroup::Base, Style->getBackgroundColor());
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));

	setModified(true);
}

bool
SensorLogger::createFromDOM(QDomElement& element)
{
	QColorGroup cgroup = monitor->colorGroup();

	title = element.attribute("title");
	frame->setTitle(title);

	cgroup.setColor(QColorGroup::Text, restoreColorFromDOM(element, "textColor", Qt::green));
	cgroup.setColor(QColorGroup::Base, restoreColorFromDOM(element, "backgroundColor", Qt::black));
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));

	logSensors.clear();

	QDomNodeList dnList = element.elementsByTagName("logsensors");
	for (uint i = 0; i < dnList.count(); i++) {
		QDomElement element = dnList.item(i).toElement();
		LogSensor* sensor = new LogSensor(monitor);
		CHECK_PTR(sensor);

		sensor->setHostName(element.attribute("hostName"));
		sensor->setSensorName(element.attribute("sensorName"));
		sensor->setFileName(element.attribute("fileName"));
		sensor->setTimerInterval(element.attribute("timerInterval").toInt());

		logSensors.append(sensor);
	}

	setModified(false);

	return (true);
}

bool
SensorLogger::addToDOM(QDomDocument& doc, QDomElement& element, bool save)
{
	element.setAttribute("title", title);

	addColorToDOM(element, "textColor", monitor->colorGroup().text());
	addColorToDOM(element, "backgroundColor", monitor->colorGroup().base());

	for (LogSensor* sensor = logSensors.first(); sensor != 0; sensor = logSensors.next())
	{
		QDomElement log = doc.createElement("logsensors");
		log.setAttribute("sensorName", sensor->getSensorName());
		log.setAttribute("hostName", sensor->getHostName());
		log.setAttribute("fileName", sensor->getFileName());
		log.setAttribute("timerInterval", sensor->getTimerInterval());

		element.appendChild(log);
	}

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
