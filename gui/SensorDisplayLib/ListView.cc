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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

    $Id$
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <qdom.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksgrd/ColorPicker.h>
#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include "ListView.h"
#include "ListView.moc"
#include "ListViewSettings.h"

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
	int val;
	static char key[100];

	strlcpy( tmp, text, sizeof(tmp) - 1 );
	number = tmp;
	for ( uint i = 0; i < strlen(tmp); i++ ) {
		number++;
		if (isdigit(tmp[i])) {
			val = atoi(number);
			tmp[i] = '\0';
			dev = tmp;

			snprintf(key, sizeof(key), "%s%016d\n", dev, val);
			return (key);
		}
	}

	strlcpy(key, text, sizeof(key) - 1);

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

	cg.setColor(QColorGroup::Link, KSGRD::Style->firstForegroundColor());
	cg.setColor(QColorGroup::Text, KSGRD::Style->secondForegroundColor());
	cg.setColor(QColorGroup::Base, KSGRD::Style->backgroundColor());

	setPalette(QPalette(cg, cg, cg));
}

void PrivateListView::update(const QString& answer)
{
	clear();

	KSGRD::SensorTokenizer lines(answer, '\n');
	for (uint i = 0; i < lines.count(); i++) {
		PrivateListViewItem *item = new PrivateListViewItem(this);
		KSGRD::SensorTokenizer records(lines[i], '\t');
		for (uint j = 0; j < records.count(); j++) {
      if ( mColumnTypes[ j ] == "f" )
        item->setText(j, KGlobal::locale()->formatNumber( records[j].toFloat() ) );
      else if ( mColumnTypes[ j ] == "D" )
        item->setText(j, KGlobal::locale()->formatNumber( records[j].toDouble(), 0 ) );
      else
			  item->setText(j, records[j]);
    }

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
	else if (type == "d" || type == "D")
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
		kdDebug(1215) << "Unknown type " << type << " of column " << label
				  << " in ListView!" << endl;
		return;
	}

  mColumnTypes.append( type );

	/* Just use some sensible default values as initial setting. */
	QFontMetrics fm = fontMetrics();
	setColumnWidth(col, fm.width(label) + 10);
}

ListView::ListView(QWidget* parent, const char* name, const QString& title, int, int)
	: KSGRD::SensorDisplay(parent, name, title)
{
	setBackgroundColor(KSGRD::Style->backgroundColor());

	monitor = new PrivateListView( frame() );
	Q_CHECK_PTR(monitor);
	monitor->setSelectionMode(QListView::NoSelection);
	monitor->setItemMargin(2);

	setMinimumSize(50, 25);

	setPlotterWidget(monitor);

	setModified(false);
}

bool
ListView::addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& title)
{
	if (sensorType != "listview")
		return (false);

	registerSensor(new KSGRD::SensorProperties(hostName, sensorName, sensorType, title));

	setTitle(title);

	/* To differentiate between answers from value requests and info
	 * requests we use 100 for info requests. */
	sendRequest(hostName, sensorName + "?", 100);

	setModified(true);
	return (true);
}

void
ListView::updateList()
{
	sendRequest(sensors().at(0)->hostName(), sensors().at(0)->name(), 19);
}

void
ListView::answerReceived(int id, const QString& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);

	switch (id)
	{
		case 100: {
			/* We have received the answer to a '?' command that contains
			 * the information about the table headers. */
			KSGRD::SensorTokenizer lines(answer, '\n');
			if (lines.count() != 2)
			{
				kdDebug(1215) << "wrong number of lines" << endl;
				return;
			}
			KSGRD::SensorTokenizer headers(lines[0], '\t');
			KSGRD::SensorTokenizer colTypes(lines[1], '\t');

			/* remove all columns from list */
			monitor->removeColumns();

			/* add the new columns */
			for (unsigned int i = 0; i < headers.count(); i++)
				/* TODO: Implement translation support for header texts */
				monitor->addColumn(headers[i], colTypes[i]);
			break;
		}
		case 19: {
			monitor->update(answer);
			break;
		}
	}
}

void
ListView::resizeEvent(QResizeEvent*)
{
	frame()->setGeometry(0, 0, width(), height());
	monitor->setGeometry(10, 20, width() - 20, height() - 30);
}

bool
ListView::restoreSettings(QDomElement& element)
{
	addSensor(element.attribute("hostName"), element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "listview" : element.attribute("sensorType")), element.attribute("title"));

	QColorGroup colorGroup = monitor->colorGroup();
	colorGroup.setColor(QColorGroup::Link, restoreColor(element, "gridColor", KSGRD::Style->firstForegroundColor()));
	colorGroup.setColor(QColorGroup::Text, restoreColor(element, "textColor", KSGRD::Style->secondForegroundColor()));
	colorGroup.setColor(QColorGroup::Base, restoreColor(element, "backgroundColor", KSGRD::Style->backgroundColor()));

	monitor->setPalette(QPalette(colorGroup, colorGroup, colorGroup));

	SensorDisplay::restoreSettings(element);

	setModified(false);

	return (true);
}

bool
ListView::saveSettings(QDomDocument& doc, QDomElement& element, bool save)
{
	element.setAttribute("hostName", sensors().at(0)->hostName());
	element.setAttribute("sensorName", sensors().at(0)->name());
	element.setAttribute("sensorType", sensors().at(0)->type());

	QColorGroup colorGroup = monitor->colorGroup();
	saveColor(element, "gridColor", colorGroup.color(QColorGroup::Link));
	saveColor(element, "textColor", colorGroup.color(QColorGroup::Text));
	saveColor(element, "backgroundColor", colorGroup.color(QColorGroup::Base));

	SensorDisplay::saveSettings(doc, element);

	if (save)
		setModified(false);

	return (true);
}

void
ListView::configureSettings()
{
	lvs = new ListViewSettings(this, "ListViewSettings", true);
	Q_CHECK_PTR(lvs);
	connect(lvs->applyButton, SIGNAL(clicked()), this, SLOT(applySettings()));

	QColorGroup colorGroup = monitor->colorGroup();
	lvs->gridColor->setColor(colorGroup.color(QColorGroup::Link));
	lvs->textColor->setColor(colorGroup.color(QColorGroup::Text));
	lvs->backgroundColor->setColor(colorGroup.color(QColorGroup::Base));
	lvs->title->setText(title());

	if (lvs->exec())
		applySettings();

	delete lvs;
	lvs = 0;
}

void
ListView::applySettings()
{
	QColorGroup colorGroup = monitor->colorGroup();
	colorGroup.setColor(QColorGroup::Link, lvs->gridColor->color());
	colorGroup.setColor(QColorGroup::Text, lvs->textColor->color());
	colorGroup.setColor(QColorGroup::Base, lvs->backgroundColor->color());
	monitor->setPalette(QPalette(colorGroup, colorGroup, colorGroup));

	setTitle(lvs->title->text());

	setModified(true);
}

void
ListView::applyStyle()
{
	QColorGroup colorGroup = monitor->colorGroup();
	colorGroup.setColor(QColorGroup::Link, KSGRD::Style->firstForegroundColor());
	colorGroup.setColor(QColorGroup::Text, KSGRD::Style->secondForegroundColor());
	colorGroup.setColor(QColorGroup::Base, KSGRD::Style->backgroundColor());
	monitor->setPalette(QPalette(colorGroup, colorGroup, colorGroup));

	setModified(true);
}
