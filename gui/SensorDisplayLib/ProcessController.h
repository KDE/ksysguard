/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!

*/

#ifndef _ProcessController_h_
#define _ProcessController_h_

#include <qdict.h>
#include <qwidget.h>

#include <kapplication.h>

#include <SensorDisplay.h>

#include "ProcessList.h"

class QVBoxLayout;
class QHBoxLayout;
class QCheckBox;
class QComboBox;
class KPushButton;

extern KApplication* Kapp;

/**
 * This widget implements a process list page. Besides the process
 * list which is implemented as a ProcessList, it contains two
 * comboxes and two buttons.  The combo boxes are used to set the
 * update rate and the process filter.  The buttons are used to force
 * an immediate update and to kill a process.
 */
class ProcessController : public KSGRD::SensorDisplay
{
	Q_OBJECT

public:
	ProcessController(QWidget* parent = 0, const char* name = 0);
	virtual ~ProcessController() { }

	void resizeEvent(QResizeEvent*);

	void clearSelection(void)
	{
		pList->clearSelection();
	}

	bool restoreSettings(QDomElement& element);

	bool saveSettings(QDomDocument& doc, QDomElement& element, bool save = true);

	void refreshList(void)
	{
		updateList();
	}

	virtual void timerEvent(QTimerEvent*)
	{
		updateList();
	}

	virtual bool addSensor(const QString&, const QString&, const QString&, const QString&);

	virtual void answerReceived(int id, const QString& answer);

	virtual void sensorError(int, bool err);

	void configureSettings() { }

	virtual bool hasSettingsDialog() const
	{
		return (false);
	}

public slots:
	void filterModeChanged(int filter)
	{
		pList->setFilterMode(filter);
		updateList();
		setModified(true);
	}

	void setTreeView(bool tv)
	{
		pList->setTreeView(tv);
		updateList();
		setModified(true);
	}

	virtual void setModified(bool mfd)
	{
		if (mfd != modified())
		{
			SensorDisplay::setModified( mfd );
			if (!mfd)
				pList->setModified(0);
			emit modified(modified());
		}
	}

	void killProcess();
	void killProcess(int pid, int sig);

	void reniceProcess(int pid, int niceValue);

	void updateList();

signals:
	void setFilterMode(int);

private:
	QVBoxLayout* gm;

	bool killSupported;

	/// The process list.
	ProcessList* pList;

	QHBoxLayout* gm1;

	/// Checkbox to switch between tree and list view
	QCheckBox* xbTreeView;

	/// This combo boxes control the process filter.
	QComboBox* cbFilter;

	/// These buttons force an immedeate refresh or kill a process.
	KPushButton* bRefresh;
	KPushButton* bKill;

	/// Dictionary for header translations.
	QDict<QString> dict;
};

#endif
