/*
    KTop, the KDE Task Manager and System Monitor
   
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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _FancyPlotter_h_
#define _FancyPlotter_h_

#include <qlabel.h>
#include <qsize.h>
#include <qgroupbox.h>

#include <kdialogbase.h>

#include "SensorDisplay.h"
#include "SignalPlotter.h"

class QListViewItem;
class FancyPlotterSettings;

class FancyPlotter : public SensorDisplay
{
	Q_OBJECT

public:
	FancyPlotter(QWidget* parent = 0, const char* name = 0,
				 const QString& title = QString::null, double min = 0,
				 double max = 100, bool noFrame = false);
	virtual ~FancyPlotter();

	void settings();

	bool addSensor(const QString& hostName, const QString& sensorName,
				   const QString& title);
	bool addSensor(const QString& hostName, const QString& sensorName,
				   const QString& title, const QColor& col);
	bool removeSensor(uint idx);

	void addSample(double s0, double s1 = 0, double s2 = 0, double s3 = 0,
				   double s4 = 0)
	{
		QValueList<double> d;
		d.append(s0);
		d.append(s1);
		d.append(s2);
		d.append(s3);
		d.append(s4);
		plotter->addSample(d);
	}

	virtual QSize sizeHint(void);

	virtual void answerReceived(int id, const QString& s);

	virtual bool createFromDOM(QDomElement& el);
	virtual bool addToDOM(QDomDocument& doc, QDomElement& display,
						  bool save = true);

	virtual bool hasBeenModified() const
	{
		return (modified);
	}

	virtual bool hasSettingsDialog() const
	{
		return (TRUE);
	}

	virtual void sensorError(int sensorID, bool err);

public slots:
	void applySettings();
	void vColorSettings();
	void hColorSettings();
	void bColorSettings();
	void settingsSetColor();
	void settingsDelete();
	void settingsSelectionChanged(QListViewItem*);

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	uint beams;
	bool modified;
	bool noFrame;

	SignalPlotter* plotter;

	FancyPlotterSettings* fps;

	/* The sample buffer and the flags are needed to store the incoming
	 * samples for each beam until all samples of the period have been
	 * received. The flags variable is used to ensure that all samples have
	 * been received. */
	QValueList<double> sampleBuf;
} ;

#endif
