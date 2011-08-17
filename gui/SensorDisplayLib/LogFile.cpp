/*
    KSysGuard, the KDE System Guard

	Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <stdio.h>
#include <sys/types.h>

#include <QPushButton>
#include <QRegExp>

#include <QFile>
#include <QListWidget>
#include <QHBoxLayout>
#include <kfontdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcolorbutton.h>
#include <knotification.h>
#include "StyleEngine.h"

#include "ui_LogFileSettings.h"

#include "LogFile.moc"

LogFile::LogFile(QWidget *parent, const QString& title, SharedSettings *workSheetSettings)
	: KSGRD::SensorDisplay(parent, title, workSheetSettings)
{
	kDebug() << "Making sensor logger";
	logFileID= 0;
	lfs = NULL;
	QLayout *layout = new QHBoxLayout(this);
	monitor = new QListWidget(this);
	layout->addWidget(monitor);
	setLayout(layout);

	setMinimumSize(50, 25);
	monitor->setContextMenuPolicy( Qt::CustomContextMenu );
	connect(monitor, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
	setPlotterWidget(monitor);
}

LogFile::~LogFile(void)
{
	sendRequest(sensors().at(0)->hostName(), QString("logfile_unregister %1" ).arg(logFileID), 43);
}

bool
LogFile::addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& title)
{
	if (sensorType != "logfile")
		return (false);

	registerSensor(new KSGRD::SensorProperties(hostName, sensorName, sensorType, title));

	QString sensorID = sensorName.right(sensorName.length() - (sensorName.lastIndexOf("/") + 1));

	sendRequest(sensors().at(0)->hostName(), QString("logfile_register %1" ).arg(sensorID), 42);

	if (title.isEmpty())
		setTitle(sensors().at(0)->hostName() + ':' + sensorID);
	else
		setTitle(title);

	return (true);
}


void LogFile::configureSettings(void)
{
	QPalette cgroup = monitor->palette();

	lfs = new Ui_LogFileSettings;
	Q_CHECK_PTR(lfs);
	KDialog dlg;
	dlg.setCaption( i18n("File logging settings") );
	dlg.setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

	lfs->setupUi(dlg.mainWidget());

	lfs->fgColor->setColor(cgroup.color( QPalette::Text ));
	lfs->fgColor->setText(i18n("Foreground color:"));
	lfs->bgColor->setColor(cgroup.color( QPalette::Base ));
	lfs->bgColor->setText(i18n("Background color:"));
	lfs->fontRequester->setFont(monitor->font());
	lfs->ruleList->addItems(filterRules);
	lfs->title->setText(title());

	connect(&dlg, SIGNAL(okClicked()), &dlg, SLOT(accept()));
	connect(&dlg, SIGNAL(applyClicked()), this, SLOT(applySettings()));

	connect(lfs->addButton, SIGNAL(clicked()), this, SLOT(settingsAddRule()));
	connect(lfs->deleteButton, SIGNAL(clicked()), this, SLOT(settingsDeleteRule()));
	connect(lfs->changeButton, SIGNAL(clicked()), this, SLOT(settingsChangeRule()));
	connect(lfs->ruleList, SIGNAL(currentRowChanged(int)), this, SLOT(settingsRuleListSelected(int)));
	connect(lfs->ruleText, SIGNAL(returnPressed()), this, SLOT(settingsAddRule()));

	if (dlg.exec()) {
		applySettings();
	}

	delete lfs;
	lfs = 0;
}

void LogFile::settingsAddRule()
{
	if (!lfs->ruleText->text().isEmpty()) {
		lfs->ruleList->addItem(lfs->ruleText->text());
		lfs->ruleText->setText("");
	}
}

void LogFile::settingsDeleteRule()
{
	delete lfs->ruleList->takeItem(lfs->ruleList->currentRow());
	lfs->ruleText->setText("");
}

void LogFile::settingsChangeRule()
{
	lfs->ruleList->currentItem()->setText(lfs->ruleText->text());
	lfs->ruleText->setText("");
}

void LogFile::settingsRuleListSelected(int index)
{
    if (index > -1)
        lfs->ruleText->setText(lfs->ruleList->item(index)->text());
}

void LogFile::applySettings(void)
{
	QPalette cgroup = monitor->palette();

	cgroup.setColor(QPalette::Text, lfs->fgColor->color());
	cgroup.setColor(QPalette::Base, lfs->bgColor->color());
	monitor->setPalette( cgroup );
	monitor->setFont(lfs->fontRequester->font());

	filterRules.clear();
	for (int i = 0; i < lfs->ruleList->count(); i++)
		filterRules.append(lfs->ruleList->item(i)->text());

	setTitle(lfs->title->text());
}

void
LogFile::applyStyle()
{
	QPalette cgroup = monitor->palette();

	cgroup.setColor(QPalette::Text, KSGRD::Style->firstForegroundColor());
	cgroup.setColor(QPalette::Base, KSGRD::Style->backgroundColor());
	monitor->setPalette( cgroup );
}

bool
LogFile::restoreSettings(QDomElement& element)
{
	QFont font;
	QPalette cgroup = monitor->palette();

	cgroup.setColor(QPalette::Active, QPalette::Text, restoreColor(element, "textColor", Qt::green));
	cgroup.setColor(QPalette::Active, QPalette::Base, restoreColor(element, "backgroundColor", Qt::black));
	cgroup.setColor(QPalette::Disabled, QPalette::Text, restoreColor(element, "textColor", Qt::green));
	cgroup.setColor(QPalette::Disabled, QPalette::Base, restoreColor(element, "backgroundColor", Qt::black));
	cgroup.setColor(QPalette::Inactive, QPalette::Text, restoreColor(element, "textColor", Qt::green));
	cgroup.setColor(QPalette::Inactive, QPalette::Base, restoreColor(element, "backgroundColor", Qt::black));
	monitor->setPalette(cgroup);

	addSensor(element.attribute("hostName"), element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "logfile" : element.attribute("sensorType")), element.attribute("title"));

	font.fromString( element.attribute( "font" ) );
	monitor->setFont(font);

	QDomNodeList dnList = element.elementsByTagName("filter");
	for (int i = 0; i < dnList.count(); i++) {
		QDomElement element = dnList.item(i).toElement();
		filterRules.append(element.attribute("rule"));
	}

	SensorDisplay::restoreSettings(element);

	return true;
}

bool
LogFile::saveSettings(QDomDocument& doc, QDomElement& element)
{
	element.setAttribute("hostName", sensors().at(0)->hostName());
	element.setAttribute("sensorName", sensors().at(0)->name());
	element.setAttribute("sensorType", sensors().at(0)->type());

	element.setAttribute("font", monitor->font().toString());

	saveColor(element, "textColor", monitor->palette().color( QPalette::Text ) );
	saveColor(element, "backgroundColor", monitor->palette().color( QPalette::Base ) );

	for (QStringList::Iterator it = filterRules.begin();
		 it != filterRules.end(); ++it)
	{
		QDomElement filter = doc.createElement("filter");
		filter.setAttribute("rule", (*it));
		element.appendChild(filter);
	}

	SensorDisplay::saveSettings(doc, element);

	return true;
}

void
LogFile::updateMonitor()
{
	sendRequest(sensors().at(0)->hostName(),
				QString("%1 %2" ).arg(sensors().at(0)->name()).arg(logFileID), 19);
}

void
LogFile::answerReceived(int id, const QList<QByteArray>& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);

	switch (id)
	{
		case 19: {
			QString s;
			for (int i = 0; i < answer.count(); i++) {
				s = QString::fromUtf8(answer[i]);
				if (monitor->count() == MAXLINES)
					monitor->takeItem(0);

				monitor->addItem(s);

				for (QStringList::Iterator it = filterRules.begin(); it != filterRules.end(); ++it) {
					QRegExp *expr = new QRegExp((*it).toLatin1());
					if (expr->indexIn(s) != -1) {
						KNotification::event("pattern_match", QString("rule '%1' matched").arg(*it),QPixmap(),this);
					}
					delete expr;
				}
			}

			monitor->setCurrentRow( monitor->count() - 1 );

			break;
		}

		case 42: {
			if(answer.isEmpty())
				logFileID= 0;
			else
				logFileID = answer[0].toULong();
			break;
		}
	}
}

