/*
    KKSysGuard, the KDE System Guard
   
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    KSysGuard is currently maintained by Chris Schlaeger
    <cs@kde.org>. Please do not commit any changes without consulting
    me first. Thanks!

    $Id$
*/

#ifndef KSG_KSYSGUARDAPPLET_H
#define KSG_KSYSGUARDAPPLET_H

#include <kpanelapplet.h>

namespace KSGRD
{
class SensorBoard;
class SensorDisplay;
}

class QDragEnterEvent;
class QDropEvent;
class QPoint;
class KSGAppletSettings;

class KSysGuardApplet : public KPanelApplet, public KSGRD::SensorBoard
{
	Q_OBJECT

  public:
    KSysGuardApplet( const QString& configFile, Type type = Normal,
                     int actions = 0, QWidget *parent = 0,
                     const char *name = 0 );
    virtual ~KSysGuardApplet();

    virtual int heightForWidth( int width ) const;
    virtual int widthForHeight( int height ) const;

    virtual void preferences();

  protected:
    void resizeEvent( QResizeEvent* );
    void dragEnterEvent( QDragEnterEvent* );
    void dropEvent( QDropEvent* );
    void customEvent( QCustomEvent* );

  private slots:
    void applySettings();

  private:
    void layout();
    void resizeDocks( uint newDockCount );
    void addEmptyDisplay( QWidget **dock, uint pos );

    bool load();
    bool save();

    int findDock( const QPoint& );
    void removeDisplay( KSGRD::SensorDisplay* );

    double mSizeRatio;
    uint mDockCount;
    KSGAppletSettings* mSettingsDlg;
    QWidget** mDockList;
};

#endif
