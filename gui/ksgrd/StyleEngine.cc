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

#include <qpushbutton.h>

#include "qspinbox.h"
#include "qimage.h"

#include "klocale.h"
#include "kconfig.h"
#include "kcolordialog.h"

#include "ColorPicker.h"
#include "StyleEngine.moc"
#include "StyleSettings.h"

using namespace KSGRD;

StyleEngine* KSGRD::Style;

StyleEngine::StyleEngine()
{
	fgColor1 = QColor(0x04fb1d);	// light green
	fgColor2 = QColor(0x04fb1d);	// light green
	alarmColor = red;
	backgroundColor = QColor(0x313031);	// almost black
	fontSize = 9;
	sensorColors.setAutoDelete(true);
	sensorColors.append(new QColor(0x1889ff));	// soft blue
	sensorColors.append(new QColor(0xff7f08));	// reddish
	sensorColors.append(new QColor(0xffeb14));	// bright yellow
	uint v = 0x00ff00;
	for (uint i = sensorColors.count(); i < 32; ++i)
	{
		v = (((v + 82) & 0xff) << 23) | (v >> 8);
		sensorColors.append(new QColor(v & 0xff, (v >> 16) & 0xff,
									   (v >> 8) & 0xff));
	}
}

StyleEngine::~StyleEngine()
{
}

void
StyleEngine::readProperties(KConfig* cfg)
{
	fgColor1 = cfg->readColorEntry("fgColor1", &fgColor1);
	fgColor2 = cfg->readColorEntry("fgColor2", &fgColor2);
	alarmColor = cfg->readColorEntry("alarmColor", &alarmColor);
	backgroundColor = cfg->readColorEntry("backgroundColor", &backgroundColor);
	fontSize = cfg->readNumEntry("fontSize", fontSize);
	QStringList sl = cfg->readListEntry("sensorColors");
	if (!sl.isEmpty())
	{
		sensorColors.clear();
		QValueListIterator<QString> it = sl.begin();
		for ( ; it != sl.end(); ++it)
			sensorColors.append(new QColor(*it));
	}
}

void
StyleEngine::saveProperties(KConfig* cfg)
{
	cfg->writeEntry("fgColor1", fgColor1);
	cfg->writeEntry("fgColor2", fgColor2);
	cfg->writeEntry("alarmColor", alarmColor);
	cfg->writeEntry("backgroundColor", backgroundColor);
	cfg->writeEntry("fontSize", fontSize);
	QStringList sl;
	QPtrListIterator<QColor> it = sensorColors;
	for ( ; it; ++it)
		sl.append((*it)->name());
	cfg->writeEntry("sensorColors", sl);
}

void
StyleEngine::configure()
{
	ss = new StyleSettings(0, "StyleSettings", true);

	ss->fgColor1->setColor(fgColor1);
	ss->fgColor2->setColor(fgColor2);
	ss->alarmColor->setColor(alarmColor);
	ss->backgroundColor->setColor(backgroundColor);
	ss->fontSize->setValue(fontSize);

	uint i;
	for (i = 0; i < sensorColors.count(); ++i)
	{
		QPixmap pm(12, 12);
		pm.fill(*sensorColors.at(i));
		ss->colorList->insertItem(pm, QString(i18n("Color %1")).arg(i));
	}

	connect(ss->changeColor, SIGNAL(clicked()),
			this, SLOT(editColor()));
	connect(ss->colorList, SIGNAL(selectionChanged(QListBoxItem*)),
			this, SLOT(selectionChanged(QListBoxItem*)));
	connect(ss->buttonApply, SIGNAL(clicked()),
			this, SLOT(applyToWorksheet()));

        connect(ss->colorList, SIGNAL(doubleClicked ( QListBoxItem * )),
                this, SLOT(editColor()));
	if (ss->exec())
		apply();

	delete ss;
}

void
StyleEngine::editColor()
{
	int ci = ss->colorList->currentItem();

	if (ci < 0)
		return;

	QColor c = ss->colorList->pixmap(ci)->convertToImage().pixel(1, 1);
	int result = KColorDialog::getColor(c);
	if (result == KColorDialog::Accepted)
	{
		QPixmap newPm(12, 12);
		newPm.fill(c);
		ss->colorList->changeItem(newPm, ss->colorList->text(ci), ci);
	}
}

void
StyleEngine::selectionChanged(QListBoxItem* lbi)
{
	ss->changeColor->setEnabled(lbi != 0);
}

void
StyleEngine::apply()
{
	fgColor1 = ss->fgColor1->getColor();
	fgColor2 = ss->fgColor2->getColor();
	alarmColor = ss->alarmColor->getColor();
	backgroundColor = ss->backgroundColor->getColor();
	fontSize = ss->fontSize->value();
	sensorColors.clear();
	for (uint i = 0; i < ss->colorList->count(); ++i)
		sensorColors.append(new QColor(ss->colorList->pixmap(i)->
									   convertToImage().pixel(1, 1)));
}
