/*
    This file is part of KSysGuard.
    Copyright ( C ) 2002 Nadeem Hasan ( nhasan@kde.org )

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or ( at your option ) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KSG_APPLETSETTINGS_H
#define KSG_APPLETSETTINGS_H

#include <kdialog.h>

class QSpinBox;

class KSGAppletSettings : public KDialog
{
  public:
    KSGAppletSettings( QWidget *parent = 0, const char *name = 0 );
    ~KSGAppletSettings();

    void setNumDisplay( int );
    int numDisplay() const;

    void setSizeRatio( int );
    int sizeRatio() const;

    void setUpdateInterval( int );
    int updateInterval() const;

  private:
    QSpinBox *mInterval;
    QSpinBox *mNumDisplay;
    QSpinBox *mSizeRatio;
};

#endif
