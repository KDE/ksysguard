/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

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

#ifndef KSG_WORKSPACE_H
#define KSG_WORKSPACE_H

#include <KTabWidget>
#include <kdirwatch.h>

class KConfig;
class KUrl;
class QString;
class WorkSheet;

class Workspace : public KTabWidget
{
  Q_OBJECT

  public:
    explicit Workspace( QWidget* parent);
    ~Workspace();

    void saveProperties( KConfigGroup& cfg );
    void readProperties( const KConfigGroup& cfg );

    bool saveOnQuit();

    bool restoreWorkSheet( const QString &fileName, bool switchToTab = true);
    QList<WorkSheet *> getWorkSheets() const { return mSheetList; }


  public Q_SLOTS:
    void newWorkSheet();
    void importWorkSheet();
    void importWorkSheet( const KUrl& );
    bool saveWorkSheet( WorkSheet *sheet );
    void exportWorkSheet();
    void exportWorkSheet( WorkSheet *sheet );
    void removeWorkSheet();
    void removeWorkSheet( const QString &fileName );
    void removeAllWorkSheets();
    void getHotNewWorksheet();
    void uploadHotNewWorksheet();
    void cut();
    void copy();
    void paste();
    void configure();
    void updateSheetTitle( QWidget* );
    void applyStyle();
    void refreshActiveWorksheet();

  private Q_SLOTS:
    virtual void contextMenu (int, const QPoint &);

  Q_SIGNALS:
    void setCaption( const QString &text);

  private:
    QList<WorkSheet *> mSheetList;

    QString makeNameForNewSheet() const;
    KDirWatch mDirWatch;
};

#endif
