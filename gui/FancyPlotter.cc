/*
    KSysGuard, the KDE Task Manager and System Monitor
   
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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <qgroupbox.h>
#include <qtextstream.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qtoolbutton.h>
#include <qdom.h>
#include <qlistview.h>
#include <qimage.h>

#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knumvalidator.h>
#include <kcolordialog.h>
#include <kdebug.h>

#include "SensorManager.h"
#include "FancyPlotterSettings.h"
#include "ColorPicker.h"
#include "FancyPlotter.moc"

FancyPlotter::FancyPlotter(QWidget* parent, const char* name,
						   const QString& title, double, double,
						   bool nf)
	: SensorDisplay(parent, name), noFrame(nf)
{
	if (!title.isEmpty())
		frame->setTitle(title);

	beams = 0;

	if (noFrame)
	{
		plotter = new SignalPlotter(this, "signalPlotter");
		plotter->topBar = true;
	}
	else
		plotter = new SignalPlotter(frame, "signalPlotter");
	CHECK_PTR(plotter);
	if (!title.isEmpty())
		plotter->setTitle(title);

	setMinimumSize(sizeHint());

	/* All RMB clicks to the plotter widget will be handled by 
	 * SensorDisplay::eventFilter. */
	plotter->installEventFilter(this);

	modified = false;
}

FancyPlotter::~FancyPlotter()
{
}

void
FancyPlotter::settings()
{
	fps = new FancyPlotterSettings(this, "FancyPlotterSettings", true);
	CHECK_PTR(fps);
	fps->title->setText(frame->title());
	fps->title->setFocus();
	fps->autoRange->setChecked(plotter->autoRange);
	fps->minVal->setText(QString("%1").arg(plotter->getMin()));
	fps->minVal->setValidator(new KFloatValidator(fps->minVal));
	fps->maxVal->setText(QString("%1").arg(plotter->getMax()));
	fps->maxVal->setValidator(new KFloatValidator(fps->maxVal));

	/* Properties for vertical lines */
	fps->vLines->setChecked(plotter->vLines);
	fps->vColor->setColor(plotter->vColor);
	fps->vDistance->setValue(plotter->vDistance);

	/* Properties for horizontal lines */
	fps->hLines->setChecked(plotter->hLines);
	fps->hColor->setColor(plotter->hColor);
	fps->hCount->setValue(plotter->hCount);

	fps->labels->setChecked(plotter->labels);
	fps->topBar->setChecked(plotter->topBar);
	fps->fontSize->setValue(plotter->fontSize);

	/* Properties for background */
	fps->bColor->setColor(plotter->bColor);

	connect(fps->applyButton, SIGNAL(clicked()),
			this, SLOT(applySettings()));

	for (uint i = 0; i < beams; ++i)
	{
		QString status = sensors.at(i)->ok ? i18n("Ok") : i18n("Error");
		QListViewItem* lvi = new QListViewItem(
			fps->sensorList, sensors.at(i)->hostName,
			SensorMgr->translateSensor(sensors.at(i)->name),
			SensorMgr->translateUnit(sensors.at(i)->unit), status);
		QPixmap pm(12, 12);
		pm.fill(plotter->beamColor[i]);
		lvi->setPixmap(1, pm);
		fps->sensorList->insertItem(lvi);
	}
	connect(fps->sColorButton, SIGNAL(clicked()),
			this, SLOT(settingsSetColor()));
	connect(fps->deleteButton, SIGNAL(clicked()),
			this, SLOT(settingsDelete()));
	connect(fps->sensorList, SIGNAL(selectionChanged(QListViewItem*)),
			this, SLOT(settingsSelectionChanged(QListViewItem*)));

	if (fps->exec())
		applySettings();

	delete fps;
	fps = 0;
}

void
FancyPlotter::applySettings()
{
	frame->setTitle(fps->title->text());
	plotter->setTitle(fps->title->text());
	if (fps->autoRange->isChecked())
		plotter->changeRange(0, 0.0, 0.0);
	else
		plotter->changeRange(0, fps->minVal->text().toDouble(),
							 fps->maxVal->text().toDouble());

	plotter->vLines = fps->vLines->isChecked();
	plotter->vColor = fps->vColor->getColor();
	plotter->vDistance = fps->vDistance->text().toUInt();

	plotter->hLines = fps->hLines->isChecked();
	plotter->hColor = fps->hColor->getColor();
	plotter->hCount = fps->hCount->text().toUInt();

	plotter->labels = fps->labels->isChecked();
	plotter->topBar = fps->topBar->isChecked();
	plotter->fontSize = fps->fontSize->text().toUInt();

	plotter->bColor = fps->bColor->getColor();

    QListViewItemIterator it(fps->sensorList);
	/* Iterate through all items of the listview and reverse iterate
	 * through the registered sensors. */
	for (int i = sensors.count() - 1; i >= 0 && i < sensors.count(); --i)
	{
		if (it.current() &&
			it.current()->text(0) == sensors.at(i)->hostName &&
			it.current()->text(1) == 
			SensorMgr->translateSensor(sensors.at(i)->name))
		{
			plotter->beamColor[i] = it.current()->pixmap(1)->
				convertToImage().pixel(1, 1);
			it++;
		}
		else
			removeSensor(i);
	}

	plotter->repaint();
	modified = true;
}

void
FancyPlotter::settingsSetColor()
{
	QListViewItem* lvi = fps->sensorList->currentItem();

	if (!lvi)
		return;

	QColor c = lvi->pixmap(1)->convertToImage().pixel(1, 1);
	int result = KColorDialog::getColor(c);
	if (result == KColorDialog::Accepted)
	{
		QPixmap newPm(12, 12);
		newPm.fill(c);
		lvi->setPixmap(1, newPm);
	}
}

void
FancyPlotter::settingsDelete()
{
	QListViewItem* lvi = fps->sensorList->currentItem();

	if (lvi)
	{
		/* Before we delete the currently selected item, we determine a
		 * new item to be selected. That way we can ensure that multiple
		 * items can be deleted without forcing the user to select a new
		 * item between the deletes. If all items are deleted, the buttons
		 * are disabled again. */
		QListViewItem* newSelected = 0;
		if (lvi->itemBelow())
		{
			lvi->itemBelow()->setSelected(true);
			newSelected = lvi->itemBelow();
		}
		else if (lvi->itemAbove())
		{
			lvi->itemAbove()->setSelected(true);
			newSelected = lvi->itemAbove();
		}
		else
			settingsSelectionChanged(0);
			
		delete lvi;

		if (newSelected)
			fps->sensorList->ensureItemVisible(newSelected);
	}
}

void
FancyPlotter::settingsSelectionChanged(QListViewItem* lvi)
{
	fps->sColorButton->setEnabled(lvi != 0);
	fps->deleteButton->setEnabled(lvi != 0);
}

void
FancyPlotter::sensorError(int sensorId, bool err)
{
	if ((uint) sensorId >= beams || sensorId < 0)
		return;

	if (err == sensors.at(sensorId)->ok)
	{
		// this happens only when the sensorOk status needs to be changed.
		sensors.at(sensorId)->ok = !err;

		bool ok = true;
		for (uint i = 0; i < beams; ++i)
			if (!sensors.at(i)->ok)
			{
				ok = false;
				break;
			}
		plotter->setSensorOk(ok);
	}
}

bool
FancyPlotter::addSensor(const QString& hostName, const QString& sensorName,
						const QString& title)
{
	return (addSensor(hostName, sensorName, title,
					  plotter->getDefaultColor(beams)));
}

bool
FancyPlotter::addSensor(const QString& hostName, const QString& sensorName,
						const QString& title, const QColor& col)
{
	if (beams > 0 && hostName != sensors.at(0)->hostName)
	{
		KMessageBox::sorry(this, QString(
						   "All sensors of this display need\n"
						   "to be from the host %1!")
						   .arg(sensors.at(0)->hostName));
		/* We have to enforce this since the answers to value requests
		 * need to be received in order. */
		return (false);
	}

	if (!plotter->addBeam(col))
		return (false);

	registerSensor(new FPSensorProperties(hostName, sensorName, title, col));

	/* To differentiate between answers from value requests and info
	 * requests we add 100 to the beam index for info requests. */
	sendRequest(hostName, sensorName + "?", beams + 100);

	++beams;
	return (true);
}

bool
FancyPlotter::removeSensor(uint idx)
{
	if (idx >= beams)
	{
		kdDebug() << "FancyPlotter::removeSensor: idx out of range ("
				  << idx << ")" << endl;
		return (false);
	}

	plotter->removeBeam(idx);
	beams--;
	SensorDisplay::removeSensor(idx);

	return (true);
}

void
FancyPlotter::resizeEvent(QResizeEvent*)
{
	if (noFrame)
		plotter->setGeometry(0, 0, width(), height());
	else
		frame->setGeometry(0, 0, width(), height());
}

QSize
FancyPlotter::sizeHint(void)
{
	if (noFrame)
		return (plotter->sizeHint());
	else
		return (frame->sizeHint());
}

void
FancyPlotter::answerReceived(int id, const QString& answer)
{
	if ((uint) id < beams)
	{
		if (id != (int) sampleBuf.count())
		{
			if (id == 0)
				sensorError(beams - 1, true);
			else
				sensorError(id - 1, true);
		}
		sampleBuf.append(answer.toDouble());
		/* We received something, so the sensor is probably ok. */
		sensorError(id, false);

		if (id == (int) beams - 1)
		{
			plotter->addSample(sampleBuf);
			sampleBuf.clear();
		}
	}
	else if (id >= 100)
	{
		SensorFloatInfo info(answer);
		if (plotter->minValue == 0.0 && plotter->maxValue == 0.0 &&
			plotter->autoRange)
		{
			/* We only use this information from the sensor when the
			 * display is still using the default values. If the
			 * sensor has been restored we don't touch the already set
			 * values. */
			plotter->changeRange(id - 100, info.getMin(), info.getMax());
		}
		sensors.at(id - 100)->unit = info.getUnit();
		timerOn();
	}
}

bool
FancyPlotter::createFromDOM(QDomElement& domElem)
{
	modified = false;

	QString title = domElem.attribute("title");
	if (!title.isEmpty())
	{
		frame->setTitle(title);
		plotter->setTitle(title);
	}

	plotter->changeRange(0, domElem.attribute("min").toDouble(),
						 domElem.attribute("max").toDouble());

	bool ok;
	plotter->vLines = domElem.attribute("vLines").toUInt(&ok);
	if (!ok)
		plotter->vLines = true;
	plotter->vColor = restoreColorFromDOM(domElem, "vColor", Qt::green);
	plotter->vDistance = domElem.attribute("vDistance").toUInt(&ok);
	if (!ok)
		plotter->vDistance = 30;

	plotter->hLines = domElem.attribute("hLines").toUInt(&ok);
	if (!ok)
		plotter->hLines = true;
	plotter->hColor = restoreColorFromDOM(domElem, "hColor", Qt::green);
	plotter->hCount = domElem.attribute("hCount").toUInt(&ok);
	if (!ok)
		plotter->hCount = 5;

	plotter->labels = domElem.attribute("labels").toUInt(&ok);
	if (!ok)
		plotter->labels = true;
	plotter->topBar = domElem.attribute("topBar").toUInt(&ok);
	if (!ok)
		plotter->topBar = false;
	plotter->fontSize = domElem.attribute("fontSize").toUInt(&ok);
	if (!ok)
		plotter->fontSize = 12;
	plotter->bColor = restoreColorFromDOM(domElem, "bColor", Qt::black);

	QDomNodeList dnList = domElem.elementsByTagName("beam");
	for (uint i = 0; i < dnList.count(); ++i)
	{
		QDomElement el = dnList.item(i).toElement();
		addSensor(el.attribute("hostName"), el.attribute("sensorName"), "",
				  restoreColorFromDOM(el, "color",
									  plotter->getDefaultColor(i)));
	}

	return (true);
}

bool
FancyPlotter::addToDOM(QDomDocument& doc, QDomElement& display, bool save)
{
	display.setAttribute("title", frame->title());
	display.setAttribute("min", plotter->getMin());
	display.setAttribute("max", plotter->getMax());
	display.setAttribute("vLines", plotter->vLines);
	addColorToDOM(display, "vColor", plotter->vColor);
	display.setAttribute("vDistance", plotter->vDistance);

	display.setAttribute("hLines", plotter->hLines);
	addColorToDOM(display, "hColor", plotter->hColor);
	display.setAttribute("hCount", plotter->hCount);

	display.setAttribute("labels", plotter->labels);
	display.setAttribute("topBar", plotter->topBar);
	display.setAttribute("fontSize", plotter->fontSize);

	addColorToDOM(display, "bColor", plotter->bColor);

	for (uint i = 0; i < beams; ++i)
	{
		QDomElement beam = doc.createElement("beam");
		display.appendChild(beam);
		beam.setAttribute("hostName", sensors.at(i)->hostName);
		beam.setAttribute("sensorName", sensors.at(i)->name);
		addColorToDOM(beam, "color", plotter->beamColor[i]);
	}
	if (save)
		modified = false;

	return (true);
}
