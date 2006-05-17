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
	: Q3ListViewItem(parent)
{
	_parent = parent;
}

int PrivateListViewItem::compare( Q3ListViewItem *item, int col, bool ascending ) const
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
    sscanf( key( col, ascending ).toLatin1(), "%d:%d", &hourPrev, &minutesPrev );
    sscanf( item->key( col, ascending ).toLatin1(), "%d:%d", &hourNext, &minutesNext );
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
        prevKey.sprintf( "%s%016d", prev.left( i ).toLatin1().constData(), prev.mid( i ).toInt() );
        break;
      }

    counter = next.length();
    for ( uint i = 0; i < counter; ++i )
      if ( next[ i ].isDigit() ) {
        nextKey.sprintf( "%s%016d", next.left( i ).toLatin1().constData(), next.mid( i ).toInt() );
        break;
      }

    return prevKey.compare( nextKey );
  } else
    return key( col, ascending ).localeAwareCompare( item->key( col, ascending ) );
}

PrivateListView::PrivateListView(QWidget *parent, const char *name)
	: Q3ListView(parent, name)
{
        QPalette cg = palette();

	cg.setColor(QPalette::Link, KSGRD::Style->firstForegroundColor());
	cg.setColor(QPalette::Text, KSGRD::Style->secondForegroundColor());
	cg.setColor(QPalette::Base, KSGRD::Style->backgroundColor());

	setPalette(QPalette(cg, cg, cg));
}

void PrivateListView::update(const QStringList& answer)
{
	setUpdatesEnabled(false);
	viewport()->setUpdatesEnabled(false);

	int vpos = verticalScrollBar()->value();
	int hpos = horizontalScrollBar()->value();

	clear();

	for (uint i = 0; i < answer.count(); i++) {
		PrivateListViewItem *item = new PrivateListViewItem(this);
		KSGRD::SensorTokenizer records(answer[i], '\t');
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

int PrivateListView::columnType( int pos ) const
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
	Q3ListView::addColumn( label );
  int col = columns() - 1;

  if (type == "s" || type == "S")
    setColumnAlignment(col, Qt::AlignLeft);
	else if (type == "d" || type == "D")
		setColumnAlignment(col, Qt::AlignRight);
	else if (type == "t")
		setColumnAlignment(col, Qt::AlignRight);
	else if (type == "f")
		setColumnAlignment(col, Qt::AlignRight);
	else if (type == "M")
		setColumnAlignment(col, Qt::AlignLeft);
	else
	{
		kDebug(1215) << "Unknown type " << type << " of column " << label
				  << " in ListView!" << endl;
		return;
	}

  mColumnTypes.append( type );

	/* Just use some sensible default values as initial setting. */
	QFontMetrics fm = fontMetrics();
	setColumnWidth(col, fm.width(label) + 10);
}

ListView::ListView(QWidget* parent, const QString& title, bool isApplet)
	: KSGRD::SensorDisplay(parent, title, isApplet)
{
	QPalette palette;
	palette.setColor(backgroundRole(), KSGRD::Style->backgroundColor());
	setPalette(palette);
	monitor = new PrivateListView( this );
	Q_CHECK_PTR(monitor);
	monitor->setSelectionMode(Q3ListView::NoSelection);
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
ListView::answerReceived(int id, const QStringList& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);

	switch (id)
	{
		case 100: {
			/* We have received the answer to a '?' command that contains
			 * the information about the table headers. */
			if (answer.count() != 2)
			{
				kDebug(1215) << "wrong number of lines" << endl;
				return;
			}
			KSGRD::SensorTokenizer headers(answer[0], '\t');
			KSGRD::SensorTokenizer colTypes(answer[1], '\t');

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
	monitor->setGeometry(10, 20, width() - 20, height() - 30);
}

bool
ListView::restoreSettings(QDomElement& element)
{
	addSensor(element.attribute("hostName"), element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "listview" : element.attribute("sensorType")), element.attribute("title"));

	QPalette pal = monitor->palette();
	pal.setColor(QPalette::Link, restoreColor(element, "gridColor",
                                                  KSGRD::Style->firstForegroundColor()));
	pal.setColor(QPalette::Text, restoreColor(element, "textColor",
                                                  KSGRD::Style->secondForegroundColor()));
	pal.setColor(QPalette::Base, restoreColor(element, "backgroundColor",
                                                  KSGRD::Style->backgroundColor()));

	monitor->setPalette( pal );

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

	QPalette pal = monitor->palette();
	saveColor(element, "gridColor", pal.color(QPalette::Link));
	saveColor(element, "textColor", pal.color(QPalette::Text));
	saveColor(element, "backgroundColor", pal.color(QPalette::Base));

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

	QPalette pal = monitor->palette();
	lvs->setGridColor(pal.color(QPalette::Link));
	lvs->setTextColor(pal.color(QPalette::Text));
	lvs->setBackgroundColor(pal.color(QPalette::Base));
	lvs->setTitle(title());

	if (lvs->exec())
		applySettings();

	delete lvs;
	lvs = 0;
}

void
ListView::applySettings()
{
	QPalette pal = monitor->palette();
	pal.setColor(QPalette::Link, lvs->gridColor());
	pal.setColor(QPalette::Text, lvs->textColor());
	pal.setColor(QPalette::Base, lvs->backgroundColor());
	monitor->setPalette(QPalette(pal, pal, pal));

	setTitle(lvs->title());

	setModified(true);
}

void
ListView::applyStyle()
{
	QPalette pal = monitor->palette();
	pal.setColor(QPalette::Link, KSGRD::Style->firstForegroundColor());
	pal.setColor(QPalette::Text, KSGRD::Style->secondForegroundColor());
	pal.setColor(QPalette::Base, KSGRD::Style->backgroundColor());
	monitor->setPalette( pal );

	setModified(true);
}
