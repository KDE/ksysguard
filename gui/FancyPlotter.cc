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
#include "FancyPlotter.moc"

static const int FrameMargin = 0;
static const int Margin = 5;
static const int HeadHeight = 10;

FancyPlotterSettings::FancyPlotterSettings(const QString& oldTitle,
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
FancyPlotterSettings::getTitle() const
{
	return (titleLE->text());
}

void
FancyPlotterSettings::applyPressed()
{
	emit applySettings(this);
}

long
FancyPlotterSettings::getMin() const
{
	return (minNI->value());
}

long
FancyPlotterSettings::getMax() const
{
	return (maxNI->value());
}

FancyPlotter::FancyPlotter(QWidget* parent, const char* name,
						   const QString& title, int min, int max)
	: SensorDisplay(parent, name)
{
	sensorNames.setAutoDelete(true);

	meterFrame = new QGroupBox(this, "meterFrame"); 
	CHECK_PTR(meterFrame);
	meterFrame->setTitle(title);

	beams = 0;

	plotter = new SignalPlotter(this, "signalPlotter", min, max);
	CHECK_PTR(plotter);

	setMinimumSize(sizeHint());

	/* All RMB clicks to the plotter widget will be handled by 
	 * SensorDisplay::eventFilter. */
	plotter->installEventFilter(this);

	modified = false;
}

FancyPlotter::~FancyPlotter()
{
	delete plotter;
	delete meterFrame;
}

void
FancyPlotter::settings()
{
	FancyPlotterSettings s(meterFrame->title(), plotter->getMin(),
						   plotter->getMax());
	connect(&s, SIGNAL(applySettings(FancyPlotterSettings*)),
			this, SLOT(applySettings(FancyPlotterSettings*)));

	if (s.exec())
		applySettings(&s);
}

void
FancyPlotter::applySettings(FancyPlotterSettings* s)
{
	meterFrame->setTitle(s->getTitle());
	plotter->changeRange(0, s->getMin(), s->getMax());
	modified = TRUE;
}

bool
FancyPlotter::addSensor(const QString& hostName, const QString& sensorName,
						const QString& title)
{
	static QColor cols[] = { blue, red, yellow, cyan, magenta };

	if ((unsigned) beams >= (sizeof(cols) / sizeof(QColor)))
		return (false);

	if (!plotter->addBeam(cols[beams]))
		return (false);

	registerSensor(hostName, sensorName);
	++beams;

	if (!title.isEmpty())
		meterFrame->setTitle(title);

	/* To differentiate between answers from value requests and info
	 * requests we add 100 to the beam index for info requests. */
	sendRequest(hostName, sensorName + "?", beams + 100);

	return (true);
}

void
FancyPlotter::resizeEvent(QResizeEvent*)
{
	meterFrame->setGeometry(0, 0, width(), height());
	QRect meter = meterFrame->contentsRect();
	meter.setX(Margin);
	meter.setY(HeadHeight + Margin);
	meter.setWidth(width() - 2 * Margin);
	meter.setHeight(height() - HeadHeight - 2 * Margin);

	plotter->setGeometry(meter);
}

QSize
FancyPlotter::sizeHint(void)
{
	QSize psize = plotter->minimumSize();
	return (QSize(psize.width() + Margin * 2,
				  psize.height() + Margin * 2 + HeadHeight));
}

void
FancyPlotter::answerReceived(int id, const QString& answer)
{
	static long s[5];

	if (id < 5)
		s[id] = answer.toLong();
	else if (id > 100)
	{
		SensorIntegerInfo info(answer);
		plotter->changeRange(id - 100, info.getMin(), info.getMax());
		timerOn();
	}

	if (id == beams - 1)
	{
		plotter->addSample(s[0], s[1], s[2], s[3], s[4]);
	}
}

bool
FancyPlotter::load(QDomElement& domElem)
{
	QString title = domElem.attribute("title");
	if (!title.isEmpty())
		meterFrame->setTitle(title);
	plotter->changeRange(0, domElem.attribute("min").toLong(),
						 domElem.attribute("max").toLong());

	QDomNodeList dnList = domElem.elementsByTagName("beam");
	for (uint i = 0; i < dnList.count(); ++i)
	{
		QDomElement el = dnList.item(i).toElement();
		addSensor(el.attribute("hostName"), el.attribute("sensorName"), "");
	}

	return (TRUE);
}

bool
FancyPlotter::save(QDomDocument& doc, QDomElement& display)
{
	display.setAttribute("title", meterFrame->title());
	display.setAttribute("min", (int) plotter->getMin());
	display.setAttribute("max", (int) plotter->getMax());

	for (int i = 0; i < beams; ++i)
	{
		QDomElement beam = doc.createElement("beam");
		display.appendChild(beam);
		beam.setAttribute("hostName", *hostNames.at(i));
		beam.setAttribute("sensorName", *sensorNames.at(i));
	}
	modified = FALSE;

	return (TRUE);
}
