/*
    KSysGuard, the KDE System Guard
   
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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
    not commit any changes without consulting me first. Thanks!

*/

#ifndef KSG_WORKSPACE_H
#define KSG_WORKSPACE_H

#include <q3ptrlist.h>
#include <qtabwidget.h>

class KConfig;
class KURL;
class QString;
class WorkSheet;

class Workspace : public QTabWidget
{
  Q_OBJECT

  public:
    Workspace( QWidget* parent, const char* name = 0 );
    ~Workspace();

    void saveProperties( KConfig* );
    void readProperties( KConfig* );

    bool saveOnQuit();

    void showProcesses();

    bool restoreWorkSheet( const QString &fileName,
                           const QString &newName = QString::null );
    void deleteWorkSheet( const QString &fileName );

  public slots:
    void newWorkSheet();
    void loadWorkSheet();
    void loadWorkSheet( const KURL& );
    void saveWorkSheet();
    void saveWorkSheet( WorkSheet *sheet );
    void saveWorkSheetAs();
    void saveWorkSheetAs( WorkSheet *sheet );
    void deleteWorkSheet();
    void removeAllWorkSheets();
    void cut();
    void copy();
    void paste();
    void configure();
    void updateCaption( QWidget* );
    void applyStyle();

  signals:
    void announceRecentURL( const KURL &url );
    void setCaption( const QString &text, bool modified );

  private:
    Q3PtrList<WorkSheet> mSheetList;

    // Directory that was used for the last load/save.
    QString mWorkDir;
    bool mAutoSave;
};

#endif
