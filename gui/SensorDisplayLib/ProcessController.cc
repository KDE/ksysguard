/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2006 John Tapsell <john.tapsell@kdemail.net>

    This program is free software; you can redistribute it and/or
    modify it under the terms version 2 of of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#include <QDomElement>
#include <QTimer>
#include <QLineEdit>
#include <QTreeView>
#include <QCheckBox>
#include <QHeaderView>

#include "ProcessController.moc"
#include "ProcessController.h"
//#define DO_MODELCHECK
#ifdef DO_MODELCHECK
#include "modeltest.h"
#endif
ProcessController::ProcessController(QWidget* parent, const QString &title, SharedSettings *workSheetSettings)
	: KSGRD::SensorDisplay(parent, title, workSheetSettings)
{
	mUi.setupUi(this);
        QTimer::singleShot(0, mUi.ksysguardprocesslist->filterLineEdit(), SLOT(setFocus()));
	setPlotterWidget(this);
	setMinimumSize(sizeHint());
}
void ProcessController::resizeEvent(QResizeEvent* ev)
{
	QWidget::resizeEvent(ev);
}

void
ProcessController::sensorError(int, bool err)
{
	if (err == sensors().at(0)->isOk())
	{
		if (!err)
		{
		} else {
			kDebug() << "SensorError called with an error" << endl;
		}
		/* This happens only when the sensorOk status needs to be changed. */
		sensors().at(0)->setIsOk( !err );
	}
	setSensorOk(sensors().at(0)->isOk());
}

bool
ProcessController::restoreSettings(QDomElement& element)
{
	bool result = addSensor(element.attribute("hostName"),
				element.attribute("sensorName"),
				(element.attribute("sensorType").isEmpty() ? "table" : element.attribute("sensorType")),
				QString());
	mUi.ksysguardprocesslist->treeView()->header()->restoreState(element.attribute("treeViewHeader").toUtf8());


	bool showTotals = element.attribute("showTotals", "1").toUInt();
	mUi.ksysguardprocesslist->setShowTotals(showTotals);

	SensorDisplay::restoreSettings(element);
	return result;
}

bool
ProcessController::saveSettings(QDomDocument& doc, QDomElement& element)
{
	element.setAttribute("hostName", sensors().at(0)->hostName());
	element.setAttribute("sensorName", sensors().at(0)->name());
	element.setAttribute("sensorType", sensors().at(0)->type());
	element.setAttribute("treeViewHeader", QString::fromUtf8(mUi.ksysguardprocesslist->treeView()->header()->saveState()));
	element.setAttribute("showTotals", mUi.ksysguardprocesslist->showTotals()?1:0);

	SensorDisplay::saveSettings(doc, element);

	return true;
}
bool ProcessController::addSensor(const QString& hostName,
                                 const QString& sensorName,
                                 const QString& sensorType,
                                 const QString& title)
{
       if (sensorType != "table")
               return false;

       registerSensor(new KSGRD::SensorProperties(hostName, sensorName, sensorType, title));
       /* This just triggers the first communication. The full set of
        * requests are send whenever the sensor reconnects (detected in
        * sensorError(). */

       sensors().at(0)->setIsOk(true); //Assume it is okay from the start
       setSensorOk(sensors().at(0)->isOk());
       return true;
}
