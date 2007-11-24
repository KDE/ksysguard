/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 2006 John Tapsell <tapsell@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 or later of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef SHARED_SETTINGS_H
#define SHARED_SETTINGS_H

/** There will be an instance of this for each WorkSheet.  A pointer will be passed to every widget on that
 *  WorkSheet.  That way the worksheet can easily update the settings for all the widgets on it.
 */
class SharedSettings 
{
  public:
	SharedSettings() { isApplet = false; locked = false; modified = false; }
	bool isApplet;
	bool locked;
	bool modified;
};

#endif
