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

#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <sys/types.h>
#include <regex.h>
#include <stdio.h>

#include "StyleEngine.h"

#include "LogFile.moc"

LogFile::LogFile(QWidget *parent, const char *name, const QString&)
	: SensorDisplay(parent, name)
{
	monitor = new QListBox(this);
	CHECK_PTR(monitor);
	
	KIconLoader iconLoader;
	QPixmap errorIcon = iconLoader.loadIcon("connect_creating",
											KIcon::Desktop, KIcon::SizeSmall);

	errorLabel = new QLabel(monitor);
	CHECK_PTR(errorLabel);

	errorLabel->setPixmap(errorIcon);
	errorLabel->resize(errorIcon.size());
	errorLabel->move(2, 2);

	frame->setTitle(title);

	/* All RMB clicks to the lcd widget will be handled by 
	 * SensorDisplay::eventFilter. */
	frame->installEventFilter(this);

	setMinimumSize(50, 25);
	setModified(false);
}

LogFile::~LogFile(void)
{
	sendRequest(sensors.at(0)->hostName, QString("logfile_unregister %1" ).arg(logFileID), 43);
}

bool
LogFile::addSensor(const QString& hostName, const QString& sensorName, const QString& t)
{
	registerSensor(new SensorProperties(hostName, sensorName, t));

	QString sensorID = sensorName.right(sensorName.length() - (sensorName.findRev("/") + 1));

	sendRequest(sensors.at(0)->hostName, QString("logfile_register %1" ).arg(sensorID), 42);

	if (t.isEmpty())
		title = sensors.at(0)->hostName + ":" + sensorID;
	else
		title = t;

	frame->setTitle(title);

	setModified(TRUE);
	return (TRUE);
}


void LogFile::settings(void)
{
	QColorGroup cgroup = monitor->colorGroup();

	lfs = new LogFileSettings(this);
	CHECK_PTR(lfs);
	
	lfs->setForegroundColor(cgroup.text());
	lfs->setBackgroundColor(cgroup.base());
	lfs->setFont(monitor->font());
	lfs->setFilterRules(filterRules);
	lfs->setTitle(title);
	
	if (lfs->exec()) {
		applySettings();
	}

	delete lfs;
	lfs = 0;
}

void LogFile::applySettings(void)
{
	QColorGroup cgroup = monitor->colorGroup();

	cgroup.setColor(QColorGroup::Text, lfs->getForegroundColor());
	cgroup.setColor(QColorGroup::Base, lfs->getBackgroundColor());
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));
	monitor->setFont(lfs->getFont());
	filterRules = lfs->getFilterRules();
	title = lfs->getTitle();

	frame->setTitle(title);

	setModified(TRUE);
}

void
LogFile::applyStyle()
{
	QColorGroup cgroup = monitor->colorGroup();

	cgroup.setColor(QColorGroup::Text, Style->getFgColor1());
	cgroup.setColor(QColorGroup::Base, Style->getBackgroundColor());
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));

	setModified(TRUE);
}

bool
LogFile::createFromDOM(QDomElement& element)
{
	QFont font;
	QColorGroup cgroup = monitor->colorGroup();

	title = element.attribute("title");

	cgroup.setColor(QColorGroup::Text, restoreColorFromDOM(element, "textColor", Qt::green));
	cgroup.setColor(QColorGroup::Base, restoreColorFromDOM(element, "backgroundColor", Qt::black));
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));

	font.setRawName(element.attribute("font"));
	monitor->setFont(font);

	QDomNodeList dnList = element.elementsByTagName("filter");
	for (uint i = 0; i < dnList.count(); i++) {
		QDomElement element = dnList.item(i).toElement();
		filterRules.append(element.attribute("rule"));
	}

	addSensor(element.attribute("hostName"), element.attribute("sensorName"), title);

	setModified(FALSE);

	return TRUE;
}

bool
LogFile::addToDOM(QDomDocument& doc, QDomElement& element, bool save)
{
	element.setAttribute("hostName", sensors.at(0)->hostName);
	element.setAttribute("sensorName", sensors.at(0)->name);

	element.setAttribute("title", title);
	element.setAttribute("font", monitor->font().rawName());

	addColorToDOM(element, "textColor", monitor->colorGroup().text());
	addColorToDOM(element, "backgroundColor", monitor->colorGroup().base());

	for (QStringList::Iterator it = filterRules.begin();
		 it != filterRules.end(); it++)
	{
		QDomElement filter = doc.createElement("filter");
		filter.setAttribute("rule", (*it));
		element.appendChild(filter);
	}

	if (save)
		setModified(FALSE);

	return TRUE;
}

void
LogFile::updateMonitor()
{
	sendRequest(sensors.at(0)->hostName,
				QString("%1 %2" ).arg(sensors.at(0)->name).arg(logFileID), 19);
}

void
LogFile::answerReceived(int id, const QString& answer)
{
	regex_t token;

	/* We received something, so the sensor is probably ok. */
	sensorError(FALSE);

	switch (id)
	{
		case 19: {
			SensorTokenizer lines(answer, '\n');

			for (uint i = 0; i < lines.numberOfTokens(); i++) {
				if (monitor->count() == MAXLINES)
					monitor->removeItem(0);

				monitor->insertItem(lines[i], -1);

				for (QStringList::Iterator it = filterRules.begin(); it != filterRules.end(); it++) {
					regcomp(&token, (*it).latin1(), REG_NEWLINE|REG_EXTENDED);
					if (!regexec(&token, lines[i].latin1(), 0, NULL, 0)) {
						/* TODO: Sent notification to event logger */
					}
					regfree(&token);
				}
			}
			break;
		}

		case 42: {
			logFileID = answer.toULong();
			break;
		}
	}
}

void
LogFile::resizeEvent(QResizeEvent*)
{
	frame->setGeometry(0, 0, this->width(), this->height());
	monitor->setGeometry(10, 20, this->width() - 20, this->height() - 30);
}

void
LogFile::sensorError(bool err)
{
	if (err == sensors.at(0)->ok) {
		// this happens only when the sensorOk status needs to be changed.
		sensors.at(0)->ok = !err;
	}

	if (err)
		errorLabel->show();
	else
		errorLabel->hide();
}
