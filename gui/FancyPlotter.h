/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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

#include <kdialogbase.h>

#include "SensorDisplay.h"
#include "SignalPlotter.h"

class FancyPlotterSettings;
class QGroupBox;

class FancyPlotter : public SensorDisplay
{
	Q_OBJECT

public:
	FancyPlotter(QWidget* parent = 0, const char* name = 0,
				 const QString& title = QString::null, double min = 0,
				 double max = 100, bool noFrame = false);
	~FancyPlotter();

	void settings();

	bool addSensor(const QString& hostName, const QString& sensorName,
				   const QString& title);

	void addSample(double s0, double s1 = 0, double s2 = 0, double s3 = 0,
				   double s4 = 0)
	{
		plotter->addSample(s0, s1, s2, s3, s4);
	}

	virtual QSize sizeHint(void);

	virtual void answerReceived(int id, const QString& s);

	virtual QString additionalWhatsThis();

	bool load(QDomElement& el);
	bool save(QDomDocument& doc, QDomElement& display);

	bool hasBeenModified() const
	{
		return (modified);
	}

	virtual bool hasSettingsDialog()
	{
		return (TRUE);
	}

	virtual void sensorError(int sensorID, bool err);

public slots:
	void applySettings();

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	int beams;
	bool modified;
	bool noFrame;

	QGroupBox* meterFrame;

	SignalPlotter* plotter;

	FancyPlotterSettings* fps;

	/* The sample buffer and the flags are needed to store the incoming
	 * samples for each beam until all samples of the period have been
	 * received. The flags variable is used to ensure that all samples have
	 * been received. */
	double sampleBuf[5];
	uint flags;
} ;

#endif
