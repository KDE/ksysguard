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

#include <stdio.h>
#include <sys/types.h>

#include <qpushbutton.h>
#include <qregexp.h>

#include <kfontdialog.h>
#include <kdebug.h>
#include <klocale.h>

#include <ksgrd/ColorPicker.h>
#include <ksgrd/StyleEngine.h>

#include "LogFile.moc"

LogFile::LogFile(QWidget *parent, const char *name, const QString& title)
	: KSGRD::SensorDisplay(parent, name, title)
{
	monitor = new QListBox(this);
	Q_CHECK_PTR(monitor);
	
	setMinimumSize(50, 25);
	setModified(false);
}

LogFile::~LogFile(void)
{
	sendRequest(sensors.at(0)->hostName, QString("logfile_unregister %1" ).arg(logFileID), 43);
}

bool
LogFile::addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& t)
{
	if (sensorType != "logfile")
		return (false);

	registerSensor(new KSGRD::SensorProperties(hostName, sensorName, sensorType, t));

	QString sensorID = sensorName.right(sensorName.length() - (sensorName.findRev("/") + 1));

	sendRequest(sensors.at(0)->hostName, QString("logfile_register %1" ).arg(sensorID), 42);

	if (t.isEmpty())
		title(sensors.at(0)->hostName + ":" + sensorID);
	else
		title(t);

	setModified(true);
	return (true);
}


void LogFile::settings(void)
{
	QColorGroup cgroup = monitor->colorGroup();

	lfs = new LogFileSettings(this);
	Q_CHECK_PTR(lfs);
	
	lfs->fgColor->setColor(cgroup.text());
	lfs->fgColor->setText(i18n("Foreground Color"));
	lfs->bgColor->setColor(cgroup.base());
	lfs->bgColor->setText(i18n("Background Color"));
	lfs->fontButton->setFont(monitor->font());
	lfs->ruleList->insertStringList(filterRules);
	lfs->title->setText(title());
	
	connect(lfs->okButton, SIGNAL(clicked()), lfs, SLOT(accept()));
	connect(lfs->applyButton, SIGNAL(clicked()), this, SLOT(applySettings()));
	connect(lfs->cancelButton, SIGNAL(clicked()), lfs, SLOT(reject()));

	connect(lfs->fontButton, SIGNAL(clicked()), this, SLOT(settingsFontSelection()));
	connect(lfs->addButton, SIGNAL(clicked()), this, SLOT(settingsAddRule()));
	connect(lfs->deleteButton, SIGNAL(clicked()), this, SLOT(settingsDeleteRule()));
	connect(lfs->changeButton, SIGNAL(clicked()), this, SLOT(settingsChangeRule()));
	connect(lfs->ruleList, SIGNAL(selected(int)), this, SLOT(settingsRuleListSelected(int)));
	connect(lfs->ruleText, SIGNAL(returnPressed()), this, SLOT(settingsAddRule()));

	if (lfs->exec()) {
		applySettings();
	}

	delete lfs;
	lfs = 0;
}

void LogFile::settingsFontSelection()
{
	QFont tmpFont = lfs->fontButton->font();

	if (KFontDialog::getFont(tmpFont) == KFontDialog::Accepted) {
		lfs->fontButton->setFont(tmpFont);
	}
}

void LogFile::settingsAddRule()
{
	if (!lfs->ruleText->text().isEmpty()) {
		lfs->ruleList->insertItem(lfs->ruleText->text(), -1);
		lfs->ruleText->setText("");
	}
}

void LogFile::settingsDeleteRule()
{
	lfs->ruleList->removeItem(lfs->ruleList->currentItem());
	lfs->ruleText->setText("");
}

void LogFile::settingsChangeRule()
{
	lfs->ruleList->changeItem(lfs->ruleText->text(), lfs->ruleList->currentItem());
	lfs->ruleText->setText("");
}

void LogFile::settingsRuleListSelected(int index)
{
	lfs->ruleText->setText(lfs->ruleList->text(index));
}

void LogFile::applySettings(void)
{
	QColorGroup cgroup = monitor->colorGroup();

	cgroup.setColor(QColorGroup::Text, lfs->fgColor->getColor());
	cgroup.setColor(QColorGroup::Base, lfs->bgColor->getColor());
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));
	monitor->setFont(lfs->fontButton->font());

	filterRules.clear();
	for (int i = 0; i < lfs->ruleList->count(); i++)
		filterRules.append(lfs->ruleList->text(i));

	title(lfs->title->text());

	setModified(true);
}

void
LogFile::applyStyle()
{
	QColorGroup cgroup = monitor->colorGroup();

	cgroup.setColor(QColorGroup::Text, KSGRD::Style->getFgColor1());
	cgroup.setColor(QColorGroup::Base, KSGRD::Style->getBackgroundColor());
	monitor->setPalette(QPalette(cgroup, cgroup, cgroup));

	setModified(true);
}

bool
LogFile::createFromDOM(QDomElement& element)
{
	QFont font;
	QColorGroup cgroup = monitor->colorGroup();

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

	internCreateFromDOM(element);

	addSensor(element.attribute("hostName"), element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "logfile" : element.attribute("sensorType")), title());

	setModified(false);

	return true;
}

bool
LogFile::addToDOM(QDomDocument& doc, QDomElement& element, bool save)
{
	element.setAttribute("hostName", sensors.at(0)->hostName);
	element.setAttribute("sensorName", sensors.at(0)->name);
	element.setAttribute("sensorType", sensors.at(0)->type);

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

	internAddToDOM(doc, element);

	if (save)
		setModified(false);

	return true;
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
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);

	switch (id)
	{
		case 19: {
			KSGRD::SensorTokenizer lines(answer, '\n');

			for (uint i = 0; i < lines.numberOfTokens(); i++) {
				if (monitor->count() == MAXLINES)
					monitor->removeItem(0);

				monitor->insertItem(lines[i], -1);

				for (QStringList::Iterator it = filterRules.begin(); it != filterRules.end(); it++) {
					QRegExp *expr = new QRegExp((*it).latin1());
					if (expr->search(lines[i].latin1()) != -1) {
						KNotifyClient::event("pattern_match", QString("rule '%1' matched").arg((*it).latin1()));
					}
					delete expr;
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
