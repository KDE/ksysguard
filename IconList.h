/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net

	Copyright (c) 1999 Chris Schlaeger
	                   cs@axys.de
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef _IconList_h_
#define _IconList_h_

#include <qlist.h>
#include <qpixmap.h>

class KtopIconListElem
{
public:
	KtopIconListElem(const char* fName, const char* iName);
	~KtopIconListElem()
	{
		delete pm;
		delete icnName;
	}

	const QPixmap* pixmap()
	{
		return (pm);
	}

	const char* name()
	{
		return (icnName);
	}

private:
	QPixmap* pm;
	char* icnName;
};

/**
 * This class loads all available mini icons. The icons can be retrieved by
 * name. The icon list is shared between all instances of this class. All
 * available icons are read in up-front. I might consider a on-demand loading
 * in the future.
 */
class KtopIconList
{
public:
	KtopIconList();
	~KtopIconList();

	/**
	 * This function can be used to retrieve icons by name.
	 */
	const QPixmap* procIcon(const char*);

private:
	/// This functions loads the icons.
	void load();

	/**
	 * The data is shared between the instances. We use this variable to
	 * keep track of how many instances have been created.
	 */
	static int instCounter;

	/// The list of loaded icons.
	static QList<KtopIconListElem>* icnList;

	/// The pixmap for the default icon.
	static const QPixmap* defaultIcon;
};

#endif
