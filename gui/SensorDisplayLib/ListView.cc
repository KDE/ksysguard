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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <qdom.h>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include "ListView.h"
#include "ListView.moc"
#include "ListViewSettings.h"

PrivateListViewItem::PrivateListViewItem(PrivateListView *parent)
	: QListViewItem(parent)
{
	_parent = parent;
}

int PrivateListViewItem::compare( QListViewItem *item, int col, bool ascending ) const
{
  int type = ((PrivateListView*)listView())->columnType( col );

  if ( type == PrivateListView::Int ) {
    int prev = (int)KGlobal::locale()->readNumber( key( col, ascending ) );
    int next = (int)KGlobal::locale()->readNumber( item->key( col, ascending ) );
    if ( prev < next )
      return -1;
    else if ( prev == next )
      return 0;
    else
      return 1;
  } else if ( type == PrivateListView::Float ) {
    double prev = KGlobal::locale()->readNumber( key( col, ascending ) );
    double next = KGlobal::locale()->readNumber( item->key( col, ascending ) );
    if ( prev < next )
      return -1;
    else
      return 1;
  } else if ( type == PrivateListView::Time ) {
    int hourPrev, hourNext, minutesPrev, minutesNext;
    sscanf( key( col, ascending ).latin1(), "%d:%d", &hourPrev, &minutesPrev );
    sscanf( item->key( col, ascending ).latin1(), "%d:%d", &hourNext, &minutesNext );
    int prev = hourPrev * 60 + minutesPrev;
    int next = hourNext * 60 + minutesNext;
    if ( prev < next )
      return -1;
    else if ( prev == next )
      return 0;
    else
      return 1;
  } else if ( type == PrivateListView::DiskStat ) {
    QString prev = key( col, ascending );
    QString next = item->key( col, ascending );
    QString prevKey, nextKey;
    
    uint counter = prev.length();
    for ( uint i = 0; i < counter; ++i )
      if ( prev[ i ].isDigit() ) {
        prevKey.sprintf( "%s%016d", prev.left( i ).latin1(), prev.mid( i ).toInt() );
        break;
      }

    counter = next.length();
    for ( uint i = 0; i < counter; ++i )
      if ( next[ i ].isDigit() ) {
        nextKey.sprintf( "%s%016d", next.left( i ).latin1(), next.mid( i ).toInt() );
        break;
      }

    return prevKey.compare( nextKey );
  } else
    return key( col, ascending ).localeAwareCompare( item->key( col, ascending ) );
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
	setUpdatesEnabled(false);
	viewport()->setUpdatesEnabled(false);

	int vpos = verticalScrollBar()->value();
	int hpos = horizontalScrollBar()->value();

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

	verticalScrollBar()->setValue(vpos);
	horizontalScrollBar()->setValue(hpos);

	viewport()->setUpdatesEnabled(true);
	setUpdatesEnabled(true);

	triggerUpdate();
}

int PrivateListView::columnType( uint pos ) const
{
  if ( pos >= mColumnTypes.count() )
    return 0;

  if ( mColumnTypes[ pos ] == "d" || mColumnTypes[ pos ] == "D" )
    return Int;
  else if ( mColumnTypes[ pos ] == "f" || mColumnTypes[ pos ] == "F" )
    return Float;
  else if ( mColumnTypes[ pos ] == "t" )
    return Time;
  else if ( mColumnTypes[ pos ] == "M" )
    return DiskStat;
  else
    return Text;
}

void PrivateListView::removeColumns(void)
{
	for (int i = columns() - 1; i >= 0; --i)
		removeColumn(i);
}

void PrivateListView::addColumn(const QString& label, const QString& type)
{
	QListView::addColumn( label );
  int col = columns() - 1;

  if (type == "s" || type == "S")
    setColumnAlignment(col, AlignLeft);
	else if (type == "d" || type == "D")
		setColumnAlignment(col, AlignRight);
	else if (type == "t")
		setColumnAlignment(col, AlignRight);
	else if (type == "f")
		setColumnAlignment(col, AlignRight);
	else if (type == "M")
		setColumnAlignment(col, AlignLeft);
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
	sendRequest(hostName, sensorName, 19);
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
	lvs = new ListViewSettings(this, "ListViewSettings");
	Q_CHECK_PTR(lvs);
	connect(lvs, SIGNAL(applyClicked()), SLOT(applySettings()));

	QColorGroup colorGroup = monitor->colorGroup();
	lvs->setGridColor(colorGroup.color(QColorGroup::Link));
	lvs->setTextColor(colorGroup.color(QColorGroup::Text));
	lvs->setBackgroundColor(colorGroup.color(QColorGroup::Base));
	lvs->setTitle(title());

	if (lvs->exec())
		applySettings();

	delete lvs;
	lvs = 0;
}

void
ListView::applySettings()
{
	QColorGroup colorGroup = monitor->colorGroup();
	colorGroup.setColor(QColorGroup::Link, lvs->gridColor());
	colorGroup.setColor(QColorGroup::Text, lvs->textColor());
	colorGroup.setColor(QColorGroup::Base, lvs->backgroundColor());
	monitor->setPalette(QPalette(colorGroup, colorGroup, colorGroup));

	setTitle(lvs->title());

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
