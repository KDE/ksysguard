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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <qcheckbox.h>
#include <qdom.h>
#include <qgroupbox.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qspinbox.h>
#include <qtextstream.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include <kapp.h>
#include <kcolordialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knumvalidator.h>

#include "ColorPicker.h"
#include "FancyPlotter.moc"
#include "FancyPlotterSettings.h"
#include "SensorManager.h"
#include "StyleEngine.h"
#include <qpushbutton.h>

FancyPlotter::FancyPlotter(QWidget* parent, const char* name,
						   const QString& title, double, double,
						   bool nf)
	: SensorDisplay(parent, name)
{
	if (!title.isEmpty())
		frame->setTitle(title);

	beams = 0;
	noFrame = nf;

	if (noFrame)
	{
		plotter = new SignalPlotter(this, "signalPlotter");
		plotter->topBar = true;
	}
	else
		plotter = new SignalPlotter(frame, "signalPlotter");
	Q_CHECK_PTR(plotter);
	if (!title.isEmpty())
		plotter->setTitle(title);

	setMinimumSize(sizeHint());

	/* All RMB clicks to the plotter widget will be handled by 
	 * SensorDisplay::eventFilter. */
	plotter->installEventFilter(this);

	setModified(false);
}

FancyPlotter::~FancyPlotter()
{
}

void
FancyPlotter::settings()
{
	fps = new FancyPlotterSettings(this, "FancyPlotterSettings", true);
	Q_CHECK_PTR(fps);
	fps->title->setText(frame->title());
	fps->title->setFocus();
	fps->autoRange->setChecked(plotter->autoRange);
	fps->minVal->setText(QString("%1").arg(plotter->getMin()));
	fps->minVal->setValidator(new KFloatValidator(fps->minVal));
	fps->maxVal->setText(QString("%1").arg(plotter->getMax()));
	fps->maxVal->setValidator(new KFloatValidator(fps->maxVal));

	/* Graph style */
	switch (plotter->graphStyle) {
		case GRAPH_ORIGINAL:
			fps->styleGroup->setButton(fps->styleGroup->id(fps->style0));
			break;
		case GRAPH_BASIC_POLY:
			fps->styleGroup->setButton(fps->styleGroup->id(fps->style1));
			break;
	}
	fps->hScale->setValue(plotter->hScale);

	/* Properties for vertical lines */
	fps->vLines->setChecked(plotter->vLines);
	fps->vColor->setColor(plotter->vColor);
	fps->vDistance->setValue(plotter->vDistance);
	fps->vScroll->setChecked(plotter->vScroll);

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
			fps->sensorList,
			QString("%1").arg(i + 1),
			sensors.at(i)->hostName,
			SensorMgr->translateSensor(sensors.at(i)->name),
			SensorMgr->translateUnit(sensors.at(i)->unit), status);
		QPixmap pm(12, 12);
		pm.fill(plotter->beamColor[i]);
		lvi->setPixmap(2, pm);
		fps->sensorList->insertItem(lvi);
	}
	connect(fps->sColorButton, SIGNAL(clicked()),
			this, SLOT(settingsSetColor()));
	connect(fps->deleteButton, SIGNAL(clicked()),
			this, SLOT(settingsDelete()));
	connect(fps->sensorList, SIGNAL(selectionChanged(QListViewItem*)),
			this, SLOT(settingsSelectionChanged(QListViewItem*)));
	connect(fps->moveUpButton, SIGNAL(clicked()),
			this, SLOT(settingsMoveUp()));
	connect(fps->moveDownButton, SIGNAL(clicked()),
			this, SLOT(settingsMoveDown()));

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
		plotter->autoRange = true;
	else {
		plotter->autoRange = false;
		plotter->changeRange(0, fps->minVal->text().toDouble(),
							 fps->maxVal->text().toDouble());
	}

	if (fps->styleGroup->selected() == fps->style0)
		plotter->graphStyle = GRAPH_ORIGINAL;
	else if (fps->styleGroup->selected() == fps->style1)
		plotter->graphStyle = GRAPH_BASIC_POLY;

	if (plotter->hScale != fps->hScale->value())
	{
		plotter->hScale = fps->hScale->value();
		// Can someone think of a useful QResizeEvent to pass?
		// It doesn't really matter anyway because it's not used.
		emit resizeEvent(0);
	}

	plotter->vLines = fps->vLines->isChecked();
	plotter->vColor = fps->vColor->getColor();
	plotter->vDistance = fps->vDistance->text().toUInt();
	plotter->vScroll = fps->vScroll->isChecked();

	plotter->hLines = fps->hLines->isChecked();
	plotter->hColor = fps->hColor->getColor();
	plotter->hCount = fps->hCount->text().toUInt();

	plotter->labels = fps->labels->isChecked();
	plotter->topBar = fps->topBar->isChecked();
	plotter->fontSize = fps->fontSize->text().toUInt();

	plotter->bColor = fps->bColor->getColor();

	/* Iterate through registered sensors and through the items of the
	 * listview. Where a sensor cannot be matched, it is removed. */
	uint delCount = 0;

	for (uint i = 0; i < sensors.count(); ++i)
	{
		bool found = false;
		QListViewItemIterator it(fps->sensorList);
		for (; it.current(); ++it)
		{
			if (it.current()->text(0) == QString("%1").arg(i + 1 + delCount))
			{
				plotter->beamColor[i] = it.current()->pixmap(2)->
																convertToImage().pixel(1, 1);
				found = true;
				if (delCount > 0) {
					it.current()->setText(0, QString("%1").arg(i + 1));
				}
				continue;
			}
		}

		if (! found)
		{
			if (removeSensor(i))
			{
				i--;
				delCount++;
			}
		}
	}

	plotter->repaint();
	setModified(true);
}

void
FancyPlotter::settingsSetColor()
{
	QListViewItem* lvi = fps->sensorList->currentItem();

	if (!lvi)
		return;

	QColor c = lvi->pixmap(2)->convertToImage().pixel(1, 1);
	int result = KColorDialog::getColor(c);
	if (result == KColorDialog::Accepted)
	{
		QPixmap newPm(12, 12);
		newPm.fill(c);
		lvi->setPixmap(2, newPm);
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

/** Moves the selected item up one */
void
FancyPlotter::settingsMoveUp()
{
	if (fps->sensorList->currentItem() != 0)
	{
		QListViewItem* temp = fps->sensorList->currentItem()->itemAbove();
		if (temp)
		{
			if (temp->itemAbove())
				fps->sensorList->currentItem()->moveItem(temp->itemAbove());
			else
				fps->sensorList->currentItem()->moveItem(temp->parent());
		}

		// Re-calculate the "sensor number" field
		QListViewItem* i = fps->sensorList->firstChild();
		for (uint count = 1; i; i = i->itemBelow(), count++)
		{
			i->setText(0, QString("%1").arg(count));
		}
	}
}

/** Moves the selected item down one */
void
FancyPlotter::settingsMoveDown()
{
	if (fps->sensorList->currentItem() != 0)
	{
		if (fps->sensorList->currentItem()->itemBelow())
		{
			fps->sensorList->currentItem()->moveItem(fps->sensorList->currentItem()->itemBelow());
		}

		// Re-calculate the "sensor number" field
		QListViewItem* i = fps->sensorList->firstChild();
		for (uint count = 1; i; i = i->itemBelow(), count++)
		{
			i->setText(0, QString("%1").arg(count));
		}
		//settingsSelectionChanged(fps->sensorList()->currentItem());
	}
}

void
FancyPlotter::settingsSelectionChanged(QListViewItem* lvi)
{
	fps->sColorButton->setEnabled(lvi != 0);
	fps->deleteButton->setEnabled(lvi != 0);
	fps->moveUpButton->setEnabled(lvi != 0 && lvi->itemAbove());
	fps->moveDownButton->setEnabled(lvi != 0 && lvi->itemBelow());
}

void
FancyPlotter::applyStyle()
{
	plotter->vColor = Style->getFgColor1();
	plotter->hColor = Style->getFgColor2();
	plotter->bColor = Style->getBackgroundColor();
	plotter->fontSize = Style->getFontSize();
	for (uint i = 0; i < plotter->beamColor.count() &&
			 i < Style->getSensorColorCount(); ++i)
		plotter->beamColor[i] = Style->getSensorColor(i);
	plotter->update();
	setModified(true);
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
					const QString& sensorType, const QString& title)
{
	return (addSensor(hostName, sensorName, sensorType, title,
					  Style->getSensorColor(beams)));
}

bool
FancyPlotter::addSensor(const QString& hostName, const QString& sensorName,
				const QString& sensorType, const QString& title, const QColor& col)
{
	if (sensorType != "integer" && sensorType != "float")
		return (false);

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

	registerSensor(new FPSensorProperties(hostName, sensorName, sensorType, title, col));

	/* To differentiate between answers from value requests and info
	 * requests we add 100 to the beam index for info requests. */
	sendRequest(hostName, sensorName + "?", beams + 100);

	++beams;

	if (noFrame)
	{
		QString tooltip;
		for (uint i = 0; i < beams; ++i)
		{
			if (i == 0)
				tooltip += QString("%1:%2").arg(sensors.at(i)->hostName).arg(sensors.at(i)->name);
			else
				tooltip += QString("\n%1:%2").arg(sensors.at(i)->hostName).arg(sensors.at(i)->name);
		}
		QToolTip::remove(plotter);
		QToolTip::add(plotter, tooltip);
	}

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

	if (noFrame)
	{
		QString tooltip;
		for (uint i = 0; i < beams; ++i)
		{
			if (i == 0)
				tooltip += QString("%1:%2").arg(sensors.at(i)->hostName).arg(sensors.at(i)->name);
			else
				tooltip += QString("\n%1:%2").arg(sensors.at(i)->hostName).arg(sensors.at(i)->name);
		}
		QToolTip::remove(plotter);
		QToolTip::add(plotter, tooltip);
	}

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
		if (plotter->autoRange)
		{
			/* We only use this information from the sensor when the
			 * display is still using the default values. If the
			 * sensor has been restored we don't touch the already set
			 * values. */
			plotter->changeRange(id - 100, info.getMin(), info.getMax());
		}
		sensors.at(id - 100)->unit = info.getUnit();
	}
}

bool
FancyPlotter::createFromDOM(QDomElement& element)
{
	QString title = element.attribute("title");
	if (!title.isEmpty())
	{
		frame->setTitle(title);
		plotter->setTitle(title);
	}

	if (element.attribute("autoRange").toInt())
		plotter->autoRange = true;
	else {
		plotter->autoRange = false;
		plotter->changeRange(0, element.attribute("min").toDouble(),
						 element.attribute("max").toDouble());
	}

	plotter->vLines = element.attribute("vLines", "1").toUInt();
	plotter->vColor = restoreColorFromDOM(element, "vColor",
										  Style->getFgColor1());
	plotter->vDistance = element.attribute("vDistance", "30").toUInt();
	plotter->vScroll = element.attribute("vScroll", "1").toUInt();

	plotter->graphStyle = element.attribute("graphStyle", "1").toUInt();
	plotter->hScale = element.attribute("hScale", "5").toUInt();

	plotter->hLines = element.attribute("hLines", "1").toUInt();
	plotter->hColor = restoreColorFromDOM(element, "hColor",
										  Style->getFgColor2());
	plotter->hCount = element.attribute("hCount", "5").toUInt();

	plotter->labels = element.attribute("labels", "1").toUInt();
	plotter->topBar = element.attribute("topBar", "0").toUInt();
	plotter->fontSize = element.attribute(
		"fontSize", QString("%1").arg(Style->getFontSize())).toUInt();

	plotter->bColor = restoreColorFromDOM(element, "bColor",
										  Style->getBackgroundColor());

	QDomNodeList dnList = element.elementsByTagName("beam");
	for (uint i = 0; i < dnList.count(); ++i)
	{
		QDomElement el = dnList.item(i).toElement();
		addSensor(el.attribute("hostName"), el.attribute("sensorName"), (el.attribute("sensorType").isEmpty() ? "integer" : el.attribute("sensorType")), "", restoreColorFromDOM(el, "color", Style->getSensorColor(i)));
	}

	internCreateFromDOM(element);

	setModified(false);

	return (true);
}

bool
FancyPlotter::addToDOM(QDomDocument& doc, QDomElement& element, bool save)
{
	element.setAttribute("title", frame->title());
	element.setAttribute("min", plotter->getMin());
	element.setAttribute("max", plotter->getMax());
	element.setAttribute("autoRange", plotter->autoRange);
	element.setAttribute("vLines", plotter->vLines);
	addColorToDOM(element, "vColor", plotter->vColor);
	element.setAttribute("vDistance", plotter->vDistance);
	element.setAttribute("vScroll", plotter->vScroll);

	element.setAttribute("graphStyle", plotter->graphStyle);
	element.setAttribute("hScale", plotter->hScale);

	element.setAttribute("hLines", plotter->hLines);
	addColorToDOM(element, "hColor", plotter->hColor);
	element.setAttribute("hCount", plotter->hCount);

	element.setAttribute("labels", plotter->labels);
	element.setAttribute("topBar", plotter->topBar);
	element.setAttribute("fontSize", plotter->fontSize);

	addColorToDOM(element, "bColor", plotter->bColor);

	for (uint i = 0; i < beams; ++i)
	{
		QDomElement beam = doc.createElement("beam");
		element.appendChild(beam);
		beam.setAttribute("hostName", sensors.at(i)->hostName);
		beam.setAttribute("sensorName", sensors.at(i)->name);
		beam.setAttribute("sensorType", sensors.at(i)->type);
		addColorToDOM(beam, "color", plotter->beamColor[i]);
	}

	internAddToDOM(doc, element);

	if (save)
		setModified(false);

	return (true);
}
