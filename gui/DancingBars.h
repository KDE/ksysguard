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

#ifndef _DancingBars_h_
#define _DancingBars_h_

#include <qlabel.h>
#include <qsize.h>
#include <qarray.h>

#include <kdialogbase.h>

#include "SensorDisplay.h"
#include "BarGraph.h"

class QGroupBox;
class QLineEdit;
class KIntNumInput;

class DancingBarsSettings : public KDialogBase
{
	Q_OBJECT

public:
	DancingBarsSettings(const QString& oldTitle, long min, long max);
	~DancingBarsSettings() { }

	QString getTitle() const;
	long getMin() const;
	long getMax() const;

signals:
	void applySettings(DancingBarsSettings*);

protected slots:
	void applyPressed();

private:
	QLineEdit* titleLE;
	KIntNumInput* minNI;
	KIntNumInput* maxNI;
	QWidget* mainWidget;
} ;

class DancingBars : public SensorDisplay
{
	Q_OBJECT

public:
	DancingBars(QWidget* parent = 0, const char* name = 0,
				 const QString& title = QString::null, int min = 0,
				 int max = 100);
	~DancingBars();

	void settings();

	bool addSensor(const QString& hostName, const QString& sensorName,
				   const QString& title);

	void updateSamples(const QArray<long>& newSamples)
	{
		plotter->updateSamples(newSamples);
	}

	virtual QSize sizeHint(void);

	virtual void answerReceived(int id, const QString& s);

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

	virtual void sensorError(bool err);

public slots:
	void applySettings(DancingBarsSettings*);

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	int bars;
	bool modified;

	QGroupBox* meterFrame;

	BarGraph* plotter;

	/* The sample buffer and the flags are needed to store the incoming
	 * samples for each beam until all samples of the period have been
	 * received. The flags variable is used to ensure that all samples have
	 * been received. */
	QArray<long> sampleBuf;
	ulong flags;
} ;

#endif
