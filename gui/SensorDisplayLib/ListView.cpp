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
#include <QMenu>
#include <QAction>

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

static QString formatByteSize(qlonglong amountInKB, int units) {
    enum { UnitsAuto, UnitsKB, UnitsMB, UnitsGB, UnitsTB, UnitsPB };
    static QString kString = i18n("%1 K", QString::fromLatin1("%1"));
    static QString mString = i18n("%1 M", QString::fromLatin1("%1"));
    static QString gString = i18n("%1 G", QString::fromLatin1("%1"));
    static QString tString = i18n("%1 T", QString::fromLatin1("%1"));
    static QString pString = i18n("%1 P", QString::fromLatin1("%1"));
    double amount;

    if (units == UnitsAuto) {
        if (amountInKB < 1024.0*0.9)
            units = UnitsKB; // amount < 0.9 MiB == KiB
        else if (amountInKB < 1024.0*1024.0*0.9)
            units = UnitsMB; // amount < 0.9 GiB == MiB
        else if (amountInKB < 1024.0*1024.0*1024.0*0.9)
            units = UnitsGB; // amount < 0.9 TiB == GiB
        else if (amountInKB < 1024.0*1024.0*1024.0*1024.0*0.9)
            units = UnitsTB; // amount < 0.9 PiB == TiB
        else
            units = UnitsPB;
    }

    switch(units) {
      case UnitsKB:
        return kString.arg(KGlobal::locale()->formatNumber(amountInKB, 0));
      case UnitsMB:
        amount = amountInKB/1024.0;
        return mString.arg(KGlobal::locale()->formatNumber(amount, 1));
      case UnitsGB:
        amount = amountInKB/(1024.0*1024.0);
        if(amount < 0.1 && amount > 0.05) amount = 0.1;
        return gString.arg(KGlobal::locale()->formatNumber(amount, 1));
      case UnitsTB:
        amount = amountInKB/(1024.0*1024.0*1024.0);
        if(amount < 0.1 && amount > 0.05) amount = 0.1;
        return tString.arg(KGlobal::locale()->formatNumber(amount, 1));
      case UnitsPB:
        amount = amountInKB/(1024.0*1024.0*1024.0*1024.0);
        if(amount < 0.1 && amount > 0.05) amount = 0.1;
        return pString.arg(KGlobal::locale()->formatNumber(amount, 1));
      default:
          return "";  // error
    }
}

ListView::ListView(QWidget* parent, const QString& title, SharedSettings *workSheetSettings)
    : KSGRD::SensorDisplay(parent, title, workSheetSettings)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    mUnits = UnitsKB;
    mView = new QTreeView(this);
//    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
//    proxyModel->setSourceModel(&mModel);
    mView->setModel(&mModel);
    mModel.setSortRole(Qt::UserRole);
    layout->addWidget(mView);
    this->setLayout(layout);

    mView->setContextMenuPolicy( Qt::CustomContextMenu );
    mView->header()->setContextMenuPolicy( Qt::CustomContextMenu );
    connect(mView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(mView->header(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(showColumnContextMenu(QPoint)));

    mView->setAlternatingRowColors(true);
    mView->header()->setMovable(true);
    mView->setSelectionMode( QAbstractItemView::NoSelection );
    mView->setUniformRowHeights(true);
    mView->setRootIsDecorated(false);
    mView->header()->setSortIndicatorShown(true);
    mView->header()->setClickable(true);
    mView->setSortingEnabled(true);

/*    QPalette palette;
    palette.setColor(backgroundRole(), KSGRD::Style->backgroundColor());
    setPalette(palette);*/

    setMinimumSize(50, 25);

    setPlotterWidget(mView);
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    mView->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
}


void ListView::showColumnContextMenu(const QPoint &point)
{
    QMenu *menu = new QMenu();

    int index = mView->header()->logicalIndexAt(point);
    if (index < 0 || index >= mColumnTypes.count())
        return; //Should be impossible

    /*if(index >= 0) {
        //selected a column.  Give the option to hide it
        action = new QAction(&menu);
        action->setData(-index-1); //We set data to be negative (and minus 1) to hide a column, and positive to show a column
        action->setText(i18n("Hide Column '%1'", d->mFilterModel.headerData(index, Qt::Horizontal, Qt::DisplayRole).toString()));
        menu.addAction(action);
        if(d->mUi->treeView->header()->sectionsHidden()) {
            menu.addSeparator();
        }
    }*/

    QAction *actionAuto = NULL;
    QAction *actionKB = NULL;
    QAction *actionMB = NULL;
    QAction *actionGB = NULL;
    QAction *actionTB = NULL;
    if (mColumnTypes[index] == KByte) {
        menu->addSeparator()->setText(i18n("Display Units"));
        QActionGroup *unitsGroup = new QActionGroup(menu);
        /* Automatic (human readable)*/
        actionAuto = new QAction(menu);
        actionAuto->setText(i18n("Mixed"));
        actionAuto->setCheckable(true);
        menu->addAction(actionAuto);
        unitsGroup->addAction(actionAuto);
        /* Kilobytes */
        actionKB = new QAction(menu);
        actionKB->setText(i18n("Kilobytes"));
        actionKB->setCheckable(true);
        menu->addAction(actionKB);
        unitsGroup->addAction(actionKB);
        /* Megabytes */
        actionMB = new QAction(menu);
        actionMB->setText(i18n("Megabytes"));
        actionMB->setCheckable(true);
        menu->addAction(actionMB);
        unitsGroup->addAction(actionMB);
        /* Gigabytes */
        actionGB = new QAction(menu);
        actionGB->setText(i18n("Gigabytes"));
        actionGB->setCheckable(true);
        menu->addAction(actionGB);
        unitsGroup->addAction(actionGB);
        /* Terabytes */
        actionTB = new QAction(menu);
        actionTB->setText(i18n("Terabytes"));
        actionTB->setCheckable(true);
        menu->addAction(actionTB);
        unitsGroup->addAction(actionTB);

        switch(mUnits) {
            case UnitsAuto:
                actionAuto->setChecked(true);
                break;
            case UnitsKB:
                actionKB->setChecked(true);
                break;
            case UnitsMB:
                actionMB->setChecked(true);
                break;
            case UnitsGB:
                actionGB->setChecked(true);
                break;
            case UnitsTB:
                actionTB->setChecked(true);
                break;
            default:
                break;
        }
        unitsGroup->setExclusive(true);
    }


    QAction *result = menu->exec(mView->header()->mapToGlobal(point));
    if (result == actionAuto)
        mUnits = UnitsAuto;
    else if (result == actionKB)
        mUnits = UnitsKB;
    else if (result == actionMB)
        mUnits = UnitsMB;
    else if (result == actionGB)
        mUnits = UnitsGB;
    else if (result == actionTB)
        mUnits = UnitsTB;
    delete menu;
}

bool
ListView::addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& title)
{
    if (sensorType != "listview")
        return false;
    if(sensorName.isEmpty())
        return false;

    registerSensor(new KSGRD::SensorProperties(hostName, sensorName, sensorType, title));

    setTitle(title);

    /* To differentiate between answers from value requests and info
     * requests we use 100 for info requests. */
    sendRequest(hostName, sensorName + '?', 100);
    sendRequest(hostName, sensorName, 19);
    return true;
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
    else if ( type == "KB" )
        return KByte;
    else if ( type == "%" )
        return Percentage;
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
                kWarning(1215) << "wrong number of lines";
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
                ColumnType type = convertColumnType(colTypes[i]);
                mColumnTypes.append(type);
                if (type == Text || type == DiskStat)
                    mModel.addColumnAlignment(Qt::AlignLeft);
                else
                    mModel.addColumnAlignment(Qt::AlignRight);
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
                for (uint j = 0; j < records.count() && j < mColumnTypes.count(); j++) {
                    QStandardItem *item = new QStandardItem();
                    item->setEditable(false);
                    switch( mColumnTypes[j] ) {
                      case Int:
                        item->setData(records[j].toLongLong(), Qt::UserRole);
                        item->setText(records[j]);
                        break;
                      case Percentage:
                        item->setData(records[j].toInt(), Qt::UserRole);
                        item->setText(records[j] + "%");
                        break;
                      case Float:
                        item->setData(records[j].toFloat(), Qt::DisplayRole);
                        item->setData(records[j].toFloat(), Qt::UserRole);
                        break;
                      case Time:
                        item->setData(QTime::fromString(records[j]), Qt::DisplayRole);
                        item->setData(QTime::fromString(records[j]), Qt::UserRole);
                        break;
                      case KByte: {
                        item->setData(records[j].toInt(), Qt::UserRole);
                        item->setText(formatByteSize(records[j].toLongLong(), mUnits));
                        break;
                      }
                      case DiskStat:
                      case Text:
                      default:
                        item->setText(records[j]);
                        item->setData(records[j], Qt::UserRole);
                    }
                    mModel.setItem(i, j, item);
                }
            }
            mModel.setRowCount(answer.count());
            mModel.sort( mView->header()->sortIndicatorSection(), mView->header()->sortIndicatorOrder() );
            break;
        }
    }
}

bool
ListView::restoreSettings(QDomElement& element)
{
    addSensor(element.attribute("hostName"), element.attribute("sensorName"), (element.attribute("sensorType").isEmpty() ? "listview" : element.attribute("sensorType")), element.attribute("title"));

    //At this stage, we don't have the heading information, so we cannot setup the headers yet.
    //Save the info, the restore later.
    mHeaderSettings = QByteArray::fromBase64(element.attribute("treeViewHeader").toLatin1());
    mUnits = (ListView::Units)element.attribute("units", "0").toInt();

/*    QPalette pal = monitor->palette();
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
    if(!sensors().isEmpty()) {
        element.setAttribute("hostName", sensors().at(0)->hostName());
        element.setAttribute("sensorName", sensors().at(0)->name());
        element.setAttribute("sensorType", sensors().at(0)->type());

/*    QPalette pal = monitor->palette();
    saveColor(element, "gridColor", pal.color(QPalette::Link));
    saveColor(element, "textColor", pal.color(QPalette::Text));
    saveColor(element, "backgroundColor", pal.color(QPalette::Base));
*/
    }
    element.setAttribute("treeViewHeader", QString::fromLatin1(mView->header()->saveState().toBase64()));
    element.setAttribute("units", QString::number(mUnits));

    SensorDisplay::saveSettings(doc, element);
    return true;
}

void
ListView::configureSettings()
{
    lvs = new ListViewSettings(this, "ListViewSettings");
    Q_CHECK_PTR(lvs);
    connect(lvs, SIGNAL(applyClicked()), SLOT(applySettings()));

/*    QPalette pal = monitor->palette();
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
/*    QPalette pal = monitor->palette();
    pal.setColor(QPalette::Link, KSGRD::Style->firstForegroundColor());
    pal.setColor(QPalette::Text, KSGRD::Style->secondForegroundColor());
    pal.setColor(QPalette::Base, KSGRD::Style->backgroundColor());
    monitor->setPalette( pal );*/
}
