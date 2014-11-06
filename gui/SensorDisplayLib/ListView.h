/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy 
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ListView_h_
#define _ListView_h_

#include <QStandardItemModel>
#include <QList>
#include <QByteArray>
#include <SensorDisplay.h>

class ListViewSettings;
class QTreeView;

class ListViewModel : public QStandardItemModel {
public:
    ListViewModel(QObject * parent = 0 ) : QStandardItemModel(parent)
    {
    }

    ListViewModel(int rows, int columns, QObject * parent = 0) : QStandardItemModel(rows, columns, parent)
    {
    }

    void addColumnAlignment( Qt::AlignmentFlag align )
    {
        mAlignment.append(align);
    }

    void clear()
    {
        QStandardItemModel::clear();
        mAlignment.clear();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        int column = index.column();

        if ( role == Qt::TextAlignmentRole && column >= 0 && column < mAlignment.size() )
            return mAlignment[column];
        else
            return QStandardItemModel::data(index, role);
    }

private:
    QList<Qt::AlignmentFlag> mAlignment;
};

class ListView : public KSGRD::SensorDisplay
{
    Q_OBJECT
public:
    ListView(QWidget* parent, const QString& title, SharedSettings *workSheetSettings);
    ~ListView() {}

    bool addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& sensorDescr);
    void answerReceived(int id, const QList<QByteArray>& answerlist);
    void updateList();

    bool restoreSettings(QDomElement& element);
    bool saveSettings(QDomDocument& doc, QDomElement& element);

    virtual bool hasSettingsDialog() const
    {
        return true;
    }

    virtual void timerTick()
    {
        updateList();
    }

    void configureSettings();

public Q_SLOTS:
    void applySettings();
    void applyStyle();
    void showColumnContextMenu(const QPoint &point);

private:

    typedef enum { Text, Int, Float, Time, DiskStat, KByte, Percentage } ColumnType;
    typedef enum { UnitsAuto, UnitsKB, UnitsMB, UnitsGB, UnitsTB, UnitsPB } Units;

    ListViewModel mModel;
    QTreeView *mView;
    ListViewSettings* lvs;
    QByteArray mHeaderSettings;
    Units mUnits;

    QList<ColumnType> mColumnTypes;
    ColumnType convertColumnType(const QString &type) const;
};

#endif
