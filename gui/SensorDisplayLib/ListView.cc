/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License version 2 or at your option version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <QDomElement>

#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <QTime>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksgrd/SensorManager.h>
#include "StyleEngine.h"

#include <QTreeView>
#include <QHeaderView>
#include "ListView.h"
#include "ListView.moc"
#include "ListViewSettings.h"

ListView::ListView(QWidget* parent, const QString& title, SharedSettings *workSheetSettings)
	: KSGRD::SensorDisplay(parent, title, workSheetSettings)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	mView = new QTreeView(this);
	//QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
//	proxyModel->setSourceModel(&mModel);
	mView->setModel(&mModel);
	layout->addWidget(mView);
	this->setLayout(layout);
	
	mView->setContextMenuPolicy( Qt::CustomContextMenu );
	connect(mView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));
	connect(mView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));

	mView->setAlternatingRowColors(true);
	mView->header()->setMovable(true);
	mView->setSelectionMode( QAbstractItemView::NoSelection );
	mView->setUniformRowHeights(true);
	mView->setRootIsDecorated(false);
	mView->header()->setSortIndicatorShown(true);
	mView->header()->setClickable(true);
	mView->setSortingEnabled(true);

/*	QPalette palette;
	palette.setColor(backgroundRole(), KSGRD::Style->backgroundColor());
	setPalette(palette);*/

	setMinimumSize(50, 25);

	setPlotterWidget(mView);
	setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	mView->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
}

bool
ListView::addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& title)
{
	if (sensorType != "listview")
		return false;
	if(sensorName.isEmpty()) return false;

	kDebug() << "addSensor and sensorName is " << sensorName;
	registerSensor(new KSGRD::SensorProperties(hostName, sensorName, sensorType, title));

	setTitle(title);

	/* To differentiate between answers from value requests and info
	 * requests we use 100 for info requests. */
	sendRequest(hostName, sensorName + '?', 100);
	sendRequest(hostName, sensorName, 19);
	return (true);
}

void ListView::updateList()
{
	for(int i = 0; i < sensors().count(); i++)
		sendRequest(sensors().at(i)->hostName(), sensors().at(i)->name(), 19);
}

ListView::ColumnType ListView::convertColumnType(const QString &type) const
{
  if ( type == "d" || type == "D" )
    return Int;
  else if ( type == "f" || type == "F" )
    return Float;
  else if ( type == "t" )
    return Time;
  else if ( type == "M" )
    return DiskStat;
  else
    return Text;
}

void
ListView::answerReceived(int id, const QList<QByteArray>& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);

	switch (id)
	{
		case 100: {
			/* We have received the answer to a '?' command that contains
			 * the information about the table headers. */
			if (answer.count() != 2)
			{
				kDebug(1215) << "wrong number of lines";
				return;
			}
			KSGRD::SensorTokenizer headers(answer[0], '\t');
			KSGRD::SensorTokenizer colTypes(answer[1], '\t');

			/* add the new columns */
			mModel.clear();
			QStringList translatedHeaders;
			for (uint i = 0; i < headers.count(); i++) {
				translatedHeaders.append( i18nc("heading from daemon", headers[i]) );
			}

			for(uint i =0 ; i < colTypes.count(); i++) {
				mColumnTypes.append(convertColumnType(colTypes[i]));
			}
			
			mModel.setHorizontalHeaderLabels(translatedHeaders);
			//If we have some header settings to restore, we can do so now
			if(!mHeaderSettings.isEmpty()) {
				mView->header()->restoreState(mHeaderSettings);
				mModel.sort( mView->header()->sortIndicatorSection(), mView->header()->sortIndicatorOrder() );
			}
			break;
		}
		case 19: {
			for (int i = 0; i < answer.count(); i++) {
				KSGRD::SensorTokenizer records(answer[i], '\t');
				for (uint j = 0; j < records.count(); j++) {
					QStandardItem *item = new QStandardItem();
					item->setEditable(false);
					switch( mColumnTypes[j] ) {
					  case Int:
						item->setData(records[j].toInt(), Qt::DisplayRole);
						break;
					  case Float:
						item->setData(records[j].toFloat(), Qt::DisplayRole);
						break;
					  case Time:
						item->setData(QTime::fromString(records[j]), Qt::DisplayRole);
						break;
					  case DiskStat:
					  case Text:
					  default:
						item->setText(records[j]);
					}
					mModel.setItem(i, j, item);
				}
			}
			mModel.setRowCount(answer.count());
			break;
		}
	}
}

bool
ListView::restoreSettings(QDomElement& element)
{
	kDebug() << "restore settings";
	addSensor(element.attribute("hostName"), element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "listview" : element.attribute("sensorType")), element.attribute("title"));

	//At this stage, we don't have the heading information, so we cannot setup the headers yet.
	//Save the info, the restore later.
	mHeaderSettings = QByteArray::fromBase64(element.attribute("treeViewHeader").toLatin1());

/*	QPalette pal = monitor->palette();
	pal.setColor(QPalette::Link, restoreColor(element, "gridColor",
                                                  KSGRD::Style->firstForegroundColor()));
	pal.setColor(QPalette::Text, restoreColor(element, "textColor",
                                                  KSGRD::Style->secondForegroundColor()));
	pal.setColor(QPalette::Base, restoreColor(element, "backgroundColor",
                                                  KSGRD::Style->backgroundColor()));

	monitor->setPalette( pal );
*/
	SensorDisplay::restoreSettings(element);

	return true;
}

bool
ListView::saveSettings(QDomDocument& doc, QDomElement& element)
{
	kDebug() << "save settings";
	if(!sensors().isEmpty()) {
		element.setAttribute("hostName", sensors().at(0)->hostName());
		element.setAttribute("sensorName", sensors().at(0)->name());
		element.setAttribute("sensorType", sensors().at(0)->type());

		kDebug() << "sensorName is " << sensors().at(0)->name();

/*	QPalette pal = monitor->palette();
	saveColor(element, "gridColor", pal.color(QPalette::Link));
	saveColor(element, "textColor", pal.color(QPalette::Text));
	saveColor(element, "backgroundColor", pal.color(QPalette::Base));
*/
	}
	element.setAttribute("treeViewHeader", QString::fromLatin1(mView->header()->saveState().toBase64()));

	SensorDisplay::saveSettings(doc, element);
	return true;
}

void
ListView::configureSettings()
{
	lvs = new ListViewSettings(this, "ListViewSettings");
	Q_CHECK_PTR(lvs);
	connect(lvs, SIGNAL(applyClicked()), SLOT(applySettings()));

/*	QPalette pal = monitor->palette();
	lvs->setGridColor(pal.color(QPalette::Link));
	lvs->setTextColor(pal.color(QPalette::Text));
	lvs->setBackgroundColor(pal.color(QPalette::Base));
	lvs->setTitle(title());
*/
	if (lvs->exec())
		applySettings();

	delete lvs;
	lvs = 0;
}

void
ListView::applySettings()
{
/*  QPalette pal = monitor->palette();
  pal.setColor(QPalette::Active, QPalette::Link, lvs->gridColor());
  pal.setColor(QPalette::Active, QPalette::Text, lvs->textColor());
  pal.setColor(QPalette::Active, QPalette::Base, lvs->backgroundColor());

  pal.setColor(QPalette::Disabled, QPalette::Link, lvs->gridColor());
  pal.setColor(QPalette::Disabled, QPalette::Text, lvs->textColor());
  pal.setColor(QPalette::Disabled, QPalette::Base, lvs->backgroundColor());

  pal.setColor(QPalette::Inactive, QPalette::Link, lvs->gridColor());
  pal.setColor(QPalette::Inactive, QPalette::Text, lvs->textColor());
  pal.setColor(QPalette::Inactive, QPalette::Base, lvs->backgroundColor());

  monitor->setPalette( pal );
*/
  setTitle(lvs->title());
}

void
ListView::applyStyle()
{
/*	QPalette pal = monitor->palette();
	pal.setColor(QPalette::Link, KSGRD::Style->firstForegroundColor());
	pal.setColor(QPalette::Text, KSGRD::Style->secondForegroundColor());
	pal.setColor(QPalette::Base, KSGRD::Style->backgroundColor());
	monitor->setPalette( pal );*/
}
