/*
    KSysGuard, the KDE System Guard
   
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

#ifndef _DancingBars_h_
#define _DancingBars_h_

#include <qlistview.h>

#include "SensorDisplay.h"
#include "BarGraph.h"

class QGroupBox;
class QLineEdit;
class KIntNumInput;
class DancingBarsSettings;

class DancingBars : public SensorDisplay
{
	Q_OBJECT

public:
	DancingBars(QWidget* parent = 0, const char* name = 0,
				const QString& title = QString::null, int min = 0,
				int max = 100, bool nf = 0);
	virtual ~DancingBars();

	void settings();

	bool addSensor(const QString& hostName, const QString& sensorName,
				   const QString& title);
	bool removeSensor(uint idx);

	void updateSamples(const QArray<double>& newSamples)
	{
		plotter->updateSamples(newSamples);
	}

	virtual QSize sizeHint(void);

	virtual void answerReceived(int id, const QString& s);

	bool createFromDOM(QDomElement& el);
	bool addToDOM(QDomDocument& doc, QDomElement& display, bool save = true);

	virtual bool hasSettingsDialog() const
	{
		return (TRUE);
	}

	virtual void sensorError(int, bool err);

public slots:
	void applySettings();
	virtual void applyStyle();
	void settingsEdit();
	void settingsDelete();
	void settingsSelectionChanged(QListViewItem*);

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	void setTitle(const QString& t);

	uint bars;

	BarGraph* plotter;

	DancingBarsSettings* dbs;

	/* The sample buffer and the flags are needed to store the incoming
	 * samples for each beam until all samples of the period have been
	 * received. The flags variable is used to ensure that all samples have
	 * been received. */
	QArray<double> sampleBuf;
	ulong flags;
	bool noFrame;
} ;

#endif
