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

#include <qgroupbox.h>
#include <qtextstream.h>
#include <qlineedit.h>
#include <qdom.h>
#include <qlayout.h>

#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include "SensorManager.h"
#include "DancingBars.moc"

DancingBarsSettings::DancingBarsSettings(const QString& oldTitle,
										   long min, long max)
	: KDialogBase(0, 0, true, QString::null, Ok | Apply | Cancel)
{
	mainWidget = new QWidget(this);
	CHECK_PTR(mainWidget);
	QVBoxLayout* vLay = new QVBoxLayout(mainWidget, 0, spacingHint());
	CHECK_PTR(vLay);

	// create input field for sheet name
	QHBoxLayout* subLay = new QHBoxLayout();
	CHECK_PTR(subLay);
	vLay->addLayout(subLay);
	QLabel* titleL = new QLabel("Title:", mainWidget, "titleL");
	CHECK_PTR(titleL);
	subLay->addWidget(titleL);
	titleLE = new QLineEdit(oldTitle, mainWidget, "titleLE");
	CHECK_PTR(titleLE);
	titleLE->setMinimumWidth(fontMetrics().maxWidth() * 20);
	subLay->addWidget(titleLE);

	// create numeric input for number of columns
	subLay = new QHBoxLayout();
	CHECK_PTR(subLay);
	vLay->addLayout(subLay);
	QLabel* minLB = new QLabel("Minimum Value:", mainWidget, "minLB");
	CHECK_PTR(minLB);
	subLay->addWidget(minLB);
	minNI = new KIntNumInput(min, mainWidget, 10, "minNI");
	CHECK_PTR(minNI);
	subLay->addWidget(minNI);

	// create numeric input for number of columns
	subLay = new QHBoxLayout();
	CHECK_PTR(subLay);
	vLay->addLayout(subLay);
	QLabel* maxLB = new QLabel("Maximum Value:", mainWidget, "maxLB");
	CHECK_PTR(maxLB);
	subLay->addWidget(maxLB);
	maxNI = new KIntNumInput(max, mainWidget, 10, "maxNI");
	CHECK_PTR(maxNI);
	subLay->addWidget(maxNI);

	vLay->addStretch(10);

	setMainWidget(mainWidget);
}

QString
DancingBarsSettings::getTitle() const
{
	return (titleLE->text());
}

void
DancingBarsSettings::applyPressed()
{
	emit applySettings(this);
}

long
DancingBarsSettings::getMin() const
{
	return (minNI->value());
}

long
DancingBarsSettings::getMax() const
{
	return (maxNI->value());
}

DancingBars::DancingBars(QWidget* parent, const char* name,
						   const QString& title, int min, int max)
	: SensorDisplay(parent, name)
{
	meterFrame = new QGroupBox(1, Qt::Vertical, title, this, "meterFrame"); 
	CHECK_PTR(meterFrame);

	bars = 0;
	flags = 0;

	plotter = new BarGraph(meterFrame, "signalPlotter", min, max);
	CHECK_PTR(plotter);

	setMinimumSize(sizeHint());

	/* All RMB clicks to the plotter widget will be handled by 
	 * SensorDisplay::eventFilter. */
	plotter->installEventFilter(this);

	modified = FALSE;
}

DancingBars::~DancingBars()
{
	delete plotter;
	delete meterFrame;
}

void
DancingBars::settings()
{
	DancingBarsSettings s(meterFrame->title(), plotter->getMin(),
						   plotter->getMax());
	connect(&s, SIGNAL(applySettings(DancingBarsSettings*)),
			this, SLOT(applySettings(DancingBarsSettings*)));

	if (s.exec())
		applySettings(&s);
}

void
DancingBars::sensorError(bool err)
{
	if (err == sensorOk)
	{
		// this happens only when the sensorOk status needs to be changed.
		meterFrame->setEnabled(!err);
		sensorOk = !err;
	}
}

void
DancingBars::applySettings(DancingBarsSettings* s)
{
	meterFrame->setTitle(s->getTitle());
	plotter->changeRange(s->getMin(), s->getMax());
	modified = TRUE;
}

bool
DancingBars::addSensor(const QString& hostName, const QString& sensorName,
						const QString& title)
{
	if (bars >= 32)
		return (false);

	if (!plotter->addBar())
		return (false);

	registerSensor(hostName, sensorName, title);
	++bars;
	sampleBuf.resize(bars);

	if (!title.isEmpty())
		meterFrame->setTitle(title);

	/* To differentiate between answers from value requests and info
	 * requests we add 100 to the beam index for info requests. */
	sendRequest(hostName, sensorName + "?", bars + 100);

	return (true);
}

void
DancingBars::resizeEvent(QResizeEvent*)
{
	meterFrame->setGeometry(0, 0, width(), height());
}

QSize
DancingBars::sizeHint(void)
{
	return (meterFrame->sizeHint());
}

void
DancingBars::answerReceived(int id, const QString& answer)
{
	if (id < 5)
	{
		sampleBuf[id] = answer.toLong();
		if (flags & (1 << id))
			qDebug("ERROR: DancingBars lost sample (%lux, %d)", flags, bars);
		flags |= 1 << id;

		if (flags == (uint) ((1 << bars) - 1))
		{
			plotter->updateSamples(sampleBuf);
			flags = 0;
		}
	}
	else if (id > 100)
	{
		SensorIntegerInfo info(answer);
		if (id == 100)
			plotter->changeRange(info.getMin(), info.getMax());
		timerOn();
	}
}

bool
DancingBars::load(QDomElement& domElem)
{
	modified = false;

	QString title = domElem.attribute("title");
	if (!title.isEmpty())
		meterFrame->setTitle(title);
	plotter->changeRange(domElem.attribute("min").toLong(),
						 domElem.attribute("max").toLong());

	QDomNodeList dnList = domElem.elementsByTagName("beam");
	for (uint i = 0; i < dnList.count(); ++i)
	{
		QDomElement el = dnList.item(i).toElement();
		addSensor(el.attribute("hostName"), el.attribute("sensorName"),
				  el.attribute("sensorDescr"));
	}

	return (TRUE);
}

bool
DancingBars::save(QDomDocument& doc, QDomElement& display)
{
	display.setAttribute("title", meterFrame->title());
	display.setAttribute("min", (int) plotter->getMin());
	display.setAttribute("max", (int) plotter->getMax());

	for (int i = 0; i < bars; ++i)
	{
		QDomElement beam = doc.createElement("beam");
		display.appendChild(beam);
		beam.setAttribute("hostName", *hostNames.at(i));
		beam.setAttribute("sensorName", *sensorNames.at(i));
		beam.setAttribute("sensorDescr", *sensorDescriptions.at(i));
	}
	modified = FALSE;

	return (TRUE);
}
