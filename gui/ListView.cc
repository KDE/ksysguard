/*
    KSysGuard, the KDE Task Manager and System Monitor

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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!
*/

#include <qdom.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kcolorbtn.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "ColorPicker.h"
#include "ListViewSettings.h"
#include "SensorManager.h"
#include "StyleEngine.h"

#include "ListView.h"
#include "ListView.moc"

const char* intKey(const char* text);
const char* timeKey(const char* text);
const char* floatKey(const char* text);
const char* diskStatKey(const char* text);

/*
 * The *key functions are used to sort the list. Since QListView can only sort
 * strings we have to massage the original contense so that the string sort
 * will produce the expected result.
 */
const char*
intKey(const char* text)
{
	int val;
	sscanf(text, "%d", &val);
	static char key[32];
	sprintf(key, "%016d", val);

	return (key);
}

const char*
timeKey(const char* text)
{
	int h, m;
	sscanf(text, "%d:%d", &h, &m);
	int t = h * 60 + m;
	static char key[32];
	sprintf(key, "%010d", t);

	return (key);
}

const char*
floatKey(const char* text)
{
	double percent;
	sscanf(text, "%lf", &percent);

	static char key[32];
	sprintf(key, "%010.2f", percent);

	return (key);
}

const char*
diskStatKey(const char* text)
{
	char *number, *dev;
	char tmp[1024];
	int i, val;
	static char key[100];

	strncpy(tmp, text, sizeof(tmp) - 1);
	number = tmp;
	for (i = 0; i < strlen(tmp); i++) {
		number++;
		if (isdigit(tmp[i])) {
			val = atoi(number);
			tmp[i] = '\0';
			dev = tmp;

			snprintf(key, sizeof(key), "%s%016d\n", dev, val);
			return (key);
		}
	}

	strncpy(key, text, sizeof(key) - 1);

	return key;
}

PrivateListViewItem::PrivateListViewItem(PrivateListView *parent)
	: QListViewItem(parent)
{
	_parent = parent;
}

QString
PrivateListViewItem::key(int column, bool) const
{
	QValueList<KeyFunc> kf = ((PrivateListView*)listView())->getSortFunc();
	KeyFunc func = *(kf.at(column));
	if (func)
		return (func(text(column).latin1()));

	return (text(column));
}

PrivateListView::PrivateListView(QWidget *parent, const char *name)
	: QListView(parent, name)
{
	QColorGroup cg = colorGroup();

	cg.setColor(QColorGroup::Link, Style->getFgColor1());
	cg.setColor(QColorGroup::Text, Style->getFgColor2());
	cg.setColor(QColorGroup::Base, Style->getBackgroundColor());

	setPalette(QPalette(cg, cg, cg));
}

void PrivateListView::update(const QString& answer)
{
	clear();

	SensorTokenizer lines(answer, '\n');
	for (uint i = 0; i < lines.numberOfTokens(); i++) {
		PrivateListViewItem *item = new PrivateListViewItem(this);
		SensorTokenizer records(lines[i], '\t');
		for (uint j = 0; j < records.numberOfTokens(); j++)
			item->setText(j, records[j]);

		insertItem(item);
	}
}

void
PrivateListView::removeColumns(void)
{
	for (int i = columns() - 1; i >= 0; --i)
		removeColumn(i);

	sortFunc.clear();
}

void
PrivateListView::addColumn(const QString& label, const QString& type)
{
	uint col = sortFunc.count();
	QListView::addColumn(label);

	if (type == "s" || type == "S")
	{
		setColumnAlignment(col, AlignLeft);
		sortFunc.append(0);
	}
	else if (type == "d")
	{
		setColumnAlignment(col, AlignRight);
		sortFunc.append(&intKey);
	}
	else if (type == "t")
	{
		setColumnAlignment(col, AlignRight);
		sortFunc.append(&timeKey);
	}
	else if (type == "f")
	{
		setColumnAlignment(col, AlignRight);
		sortFunc.append(&floatKey);
	}
	/* special sort function for partitions/list sensor */
	else if (type == "M")
	{
		setColumnAlignment(col, AlignLeft);
		sortFunc.append(&diskStatKey);
	}
	else
	{
		kdDebug() << "Unknown type " << type << " of column " << label
				  << " in ListView!" << endl;
		return;
	}

	/* Just use some sensible default values as initial setting. */
	QFontMetrics fm = fontMetrics();
	setColumnWidth(col, fm.width(label) + 10);
}

ListView::ListView(QWidget* parent, const char* name, const QString& t,
				   int, int) : SensorDisplay(parent, name)
{
	setBackgroundColor(Style->getBackgroundColor());

	monitor = new PrivateListView(frame);
	Q_CHECK_PTR(monitor);
	monitor->setSelectionMode(QListView::NoSelection);
	monitor->setItemMargin(2);

	KIconLoader iconLoader;
	QPixmap errorIcon = iconLoader.loadIcon("connect_creating", KIcon::Desktop, KIcon::SizeSmall);

	errorLabel = new QLabel(monitor);
	Q_CHECK_PTR(errorLabel);

	errorLabel->setPixmap(errorIcon);
	errorLabel->resize(errorIcon.size());
	errorLabel->move(2, 2);

	title = t;
	frame->setTitle(title);

	setMinimumSize(50, 25);
}

bool
ListView::addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& title)
{
	if (sensorType != "listview")
		return (false);

	registerSensor(new SensorProperties(hostName, sensorName, sensorType, title));

	frame->setTitle(title);

	/* To differentiate between answers from value requests and info
	 * requests we use 100 for info requests. */
	sendRequest(hostName, sensorName + "?", 100);

	setModified(TRUE);
	return (TRUE);
}

void
ListView::updateList()
{
	sendRequest(sensors.at(0)->hostName, sensors.at(0)->name, 19);
}

void
ListView::answerReceived(int id, const QString& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(false);

	switch (id)
	{
		case 100: {
			/* We have received the answer to a '?' command that contains
			 * the information about the table headers. */
			SensorTokenizer lines(answer, '\n');
			if (lines.numberOfTokens() != 2)
			{
				kdDebug() << "wrong number of lines" << endl;
				return;
			}
			SensorTokenizer headers(lines[0], '\t');
			SensorTokenizer colTypes(lines[1], '\t');

			/* remove all columns from list */
			monitor->removeColumns();

			/* add the new columns */
			for (unsigned int i = 0; i < headers.numberOfTokens(); i++)
				/* TODO: Implement translation support for header texts */
				monitor->addColumn(headers[i], colTypes[i]);

			timerOn();
			break;
		}
		case 19: {
			monitor->update(answer);
			timerOn();
			break;
		}
	}
}

void
ListView::resizeEvent(QResizeEvent*)
{
	frame->setGeometry(0, 0, width(), height());
	monitor->setGeometry(10, 20, width() - 20, height() - 30);
}

bool
ListView::createFromDOM(QDomElement& element)
{
	title = element.attribute("title");

	addSensor(element.attribute("hostName"), element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "listview" : element.attribute("sensorType")), title);

	QColorGroup colorGroup = monitor->colorGroup();
	colorGroup.setColor(QColorGroup::Link, restoreColorFromDOM(element, "gridColor", Style->getFgColor1()));
	colorGroup.setColor(QColorGroup::Text, restoreColorFromDOM(element, "textColor", Style->getFgColor2()));
	colorGroup.setColor(QColorGroup::Base, restoreColorFromDOM(element, "backgroundColor", Style->getBackgroundColor()));

	monitor->setPalette(QPalette(colorGroup, colorGroup, colorGroup));

	internCreateFromDOM(element);

	setModified(FALSE);

	return (TRUE);
}

bool
ListView::addToDOM(QDomDocument& doc, QDomElement& element, bool save)
{
	element.setAttribute("hostName", sensors.at(0)->hostName);
	element.setAttribute("sensorName", sensors.at(0)->name);
	element.setAttribute("sensorType", sensors.at(0)->type);
	element.setAttribute("title", title);

	QColorGroup colorGroup = monitor->colorGroup();
	addColorToDOM(element, "gridColor", colorGroup.color(QColorGroup::Link));
	addColorToDOM(element, "textColor", colorGroup.color(QColorGroup::Text));
	addColorToDOM(element, "backgroundColor", colorGroup.color(QColorGroup::Base));

	internAddToDOM(doc, element);

	if (save)
		setModified(FALSE);

	return (TRUE);
}

void
ListView::settings()
{
	lvs = new ListViewSettings(this, "ListViewSettings", TRUE);
	Q_CHECK_PTR(lvs);
	connect(lvs->applyButton, SIGNAL(clicked()), this, SLOT(applySettings()));

	QColorGroup colorGroup = monitor->colorGroup();
	lvs->gridColor->setColor(colorGroup.color(QColorGroup::Link));
	lvs->textColor->setColor(colorGroup.color(QColorGroup::Text));
	lvs->backgroundColor->setColor(colorGroup.color(QColorGroup::Base));
	lvs->title->setText(title);

	if (lvs->exec())
		applySettings();

	delete lvs;
	lvs = 0;
}

void
ListView::applySettings()
{
	QColorGroup colorGroup = monitor->colorGroup();
	colorGroup.setColor(QColorGroup::Link, lvs->gridColor->getColor());
	colorGroup.setColor(QColorGroup::Text, lvs->textColor->getColor());
	colorGroup.setColor(QColorGroup::Base, lvs->backgroundColor->getColor());
	monitor->setPalette(QPalette(colorGroup, colorGroup, colorGroup));

	title = lvs->title->text();
	frame->setTitle(title);

	setModified(TRUE);
}

void
ListView::applyStyle()
{
	QColorGroup colorGroup = monitor->colorGroup();
	colorGroup.setColor(QColorGroup::Link, Style->getFgColor1());
	colorGroup.setColor(QColorGroup::Text, Style->getFgColor2());
	colorGroup.setColor(QColorGroup::Base, Style->getBackgroundColor());
	monitor->setPalette(QPalette(colorGroup, colorGroup, colorGroup));

	setModified(TRUE);
}

void
ListView::sensorError(bool err)
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

