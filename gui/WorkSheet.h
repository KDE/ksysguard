/*
    KKSysGuard, the KDE System Guard
   
	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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

#ifndef _WorkSheet_h_
#define _WorkSheet_h_

#include <qwidget.h>
#include <qvaluelist.h>

class QGridLayout;
class QDragEnterEvent;
class QDropEvent;
class QString;
class QDomElement;
class QTextStream;
class SensorDisplay;

/**
 * A WorkSheet contains the displays to visualize the sensor results. When
 * creating the WorkSheet you must specify the number of columns. Displays
 * can be added and removed on the fly. The grid layout will handle the
 * layout. The number of columns can not be changed. Displays are added by
 * dragging a sensor from the sensor browser over the WorkSheet.
 */
class WorkSheet : public QWidget
{
	Q_OBJECT
public:
	WorkSheet(QWidget* parent);
	WorkSheet(QWidget* parent, uint rows, uint columns, uint interval);
	~WorkSheet();

	bool load(const QString& fN);
	bool save(const QString& fN);

	void cut();
	void copy();
	void paste();

	const QString& getFileName() const
	{
		return (fileName);
	}

	void setFileName(const QString& fN)
	{
		fileName = fN;
		setModified(true);
	}

	bool hasBeenModified() const
	{
		return modified;
	}

	SensorDisplay* addDisplay(const QString& hostname,
							  const QString& monitor,
							  const QString& sensorType,
							  const QString& sensorDescr, uint r, uint c);
	void settings();

	uint updateInterval;

public slots:
	void showPopupMenu(SensorDisplay* display);
	void setModified(bool mfd);
	void applyStyle();

signals:
	void sheetModified(QWidget* sheet);

protected:
	void dragEnterEvent(QDragEnterEvent* ev);
	void dropEvent(QDropEvent* ev);
	void customEvent(QCustomEvent* ev);

private:
	void removeDisplay(SensorDisplay* display);

	bool replaceDisplay(uint r, uint c, QDomElement& element);

	void replaceDisplay(uint r, uint c, SensorDisplay* display = 0);

	void collectHosts(QValueList<QString>& l);

	void createGrid(uint r, uint c);

	void resizeGrid(uint r, uint c);

	SensorDisplay* currentDisplay(uint* r = 0, uint* c = 0);

	void fixTabOrder();

	QString currentDisplayAsXML();

	QString fileName;
	bool modified;
	uint rows;
	uint columns;
	QGridLayout* lm;
	/* This two dimensional array stores the pointers to the sensor displays
	 * or if no sensor is present at a position a pointer to a dummy widget.
	 * The size of the array corresponds to the size of the grid layout. */
	SensorDisplay*** displays;
} ;

#endif
