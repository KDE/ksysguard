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

#include <stdio.h>
#include <qdom.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpainter.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kcolorbtn.h>

#include "ColorPicker.h"
#include "ListViewSettings.h"
#include "SensorManager.h"
#include "StyleEngine.h"

#include "ListView.moc"

ListViewItem::ListViewItem(MyListView *parent)
	: QListViewItem( parent )
{
	gridColor = (QColor)parent->getGridColor();
	textColor = (QColor)parent->getTextColor();
	backgroundColor = (QColor)parent->getBackgroundColor();
}

void 
ListViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column,
						int width, int alignment)
{
	QColorGroup colorGroup(cg);

	colorGroup.setColor(QColorGroup::Text, textColor);
	colorGroup.setColor(QColorGroup::Base, backgroundColor);

	QListViewItem::paintCell(p, colorGroup, column, width, alignment);
	p->setPen((QColor)gridColor);
	p->drawLine(0, height() - 1, width - 1, height() - 1);
}

void
ListViewItem::paintFocus(QPainter *, const QColorGroup &, const QRect &)
{
	// dummy function
}

MyListView::MyListView(QWidget *parent)
	: QListView(parent)
{
	gridColor = QColor(Qt::green);
	textColor = QColor(Qt::green);
	backgroundColor = QColor(Qt::black);

	QColorGroup cg = this->colorGroup();
	cg.setBrush(QColorGroup::Base, backgroundColor);

	QPalette pal(cg, cg, cg);
	
	this->setPalette(pal);
}

ListView::ListView(QWidget* parent, const char* name, const QString& t,
				   int, int) : SensorDisplay(parent, name)
{
	monitor = new MyListView(frame);
	CHECK_PTR(monitor);
	monitor->setBackgroundColor(Qt::black);
	monitor->setSelectionMode(QListView::NoSelection);
	monitor->setItemMargin(2);

	KIconLoader iconLoader;
	QPixmap errorIcon = iconLoader.loadIcon("connect_creating",
											KIcon::Desktop, KIcon::SizeSmall);

	errorLabel = new QLabel(monitor);
	CHECK_PTR(errorLabel);

	errorLabel->setPixmap(errorIcon);
	errorLabel->resize(errorIcon.size());
	errorLabel->move(2, 2);

	title = t;
	frame->setTitle(title);

	setMinimumSize(50, 25);
}

bool
ListView::addSensor(const QString& hostName, const QString& sensorName, const QString& title)
{
	registerSensor(new SensorProperties(hostName, sensorName, title));

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
			SensorTokenizer headers(lines[0], '\t');
		
			/* Remove all columns from list */
			for (int i = monitor->columns() - 1; i >= 0; --i) {
				monitor->removeColumn(i);
			}

			int width = (monitor->width() / headers.numberOfTokens() - 1);

			/* Add the new columns */
			for (unsigned int i = 0; i < headers.numberOfTokens(); i++)
			{
				monitor->addColumn(headers[i], -1);
				monitor->setColumnWidthMode(i, QListView::Maximum);
				if (i == (headers.numberOfTokens() - 1)) {
					width -= 15;
				}
				monitor->setColumnWidth(i, width);
			}

			timerOn();
			break;
		}
		case 19: {
			SensorTokenizer lines(answer, '\n');

			monitor->clear();
			for (unsigned int i = 0; i < lines.numberOfTokens(); i++) {
				ListViewItem *item = new ListViewItem(monitor);
				SensorTokenizer records(lines[i], '\t');
				for (unsigned int j = 0; j < records.numberOfTokens(); j++) {
					item->setText(j, records[j]);
				}
				monitor->insertItem(item);
			}

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

	monitor->setGridColor(restoreColorFromDOM(element, "gridColor",
											   Qt::green));
	monitor->setTextColor(restoreColorFromDOM(element, "textColor",
											   Qt::green));
	monitor->setBackgroundColor(
		restoreColorFromDOM(element, "backgroundColor", Qt::black));
	addSensor(element.attribute("hostName"),
			  element.attribute("sensorName"), title);

	QColorGroup colorGroup = monitor->colorGroup();
	colorGroup.setBrush(QColorGroup::Base,
						restoreColorFromDOM(element, "backgroundColor",
											Qt::black));
	QPalette pal(colorGroup, colorGroup, colorGroup);
	monitor->setPalette(pal);

	setModified(FALSE);

	return (TRUE);
}

bool
ListView::addToDOM(QDomDocument&, QDomElement& element, bool save)
{
	element.setAttribute("hostName", sensors.at(0)->hostName);
	element.setAttribute("sensorName", sensors.at(0)->name);
	element.setAttribute("title", title);

	addColorToDOM(element, "gridColor", monitor->getGridColor());
	addColorToDOM(element, "textColor", monitor->getTextColor());
	addColorToDOM(element, "backgroundColor", monitor->getBackgroundColor());

	if (save)
		setModified(FALSE);

	return (TRUE);
}

void
ListView::settings()
{
	lvs = new ListViewSettings(this, "ListViewSettings", TRUE);
	CHECK_PTR(lvs);
	connect(lvs->applyButton, SIGNAL(clicked()), this, SLOT(applySettings()));

	lvs->gridColor->setColor(monitor->getGridColor());
	lvs->textColor->setColor(monitor->getTextColor());
	lvs->backgroundColor->setColor(monitor->getBackgroundColor());
	lvs->title->setText(title);

	if (lvs->exec())
		applySettings();

	delete lvs;
	lvs = 0;
}

void
ListView::applySettings()
{
	monitor->setGridColor(lvs->gridColor->getColor());
	monitor->setTextColor(lvs->textColor->getColor());
	monitor->setBackgroundColor(lvs->backgroundColor->getColor());
	title = lvs->title->text();
	frame->setTitle(title);

	setModified(TRUE);
}

void
ListView::applyStyle()
{
	monitor->setGridColor(Style->getFgColor1());
	monitor->setTextColor(Style->getFgColor2());
	monitor->setBackgroundColor(Style->getBackgroundColor());

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
