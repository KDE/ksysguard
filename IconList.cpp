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

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include "IconList.h"
#include "ktop.h"

#define DEBUG_MODE

/*
 * Default Icon (exec.xpm)
 * Drawn by Mark Donohoe for the K Desktop Environment.
 */
static const char* execXpm[]=
{
	"16 16 7 1",
	"# c #000000",
	"d c #008080",
	"a c #beffff",
	"c c #00c0c0",
	"b c #585858",
	"e c #00c0c0",
	". c None",
	"................",
	".....######.....",
	"..###abcaba###..",
	".#cabcbaababaa#.",
	".#cccaaccaacaa#.",
	".###accaacca###.",
	"#ccccca##aaccaa#",
	"#dd#ecc##cca#cc#",
	"#d#dccccccacc#c#",
	"##adcccccccccd##",
	".#ad#c#cc#d#cd#.",
	".#a#ad#cc#dd#d#.",
	".###ad#dd#dd###.",
	"...#ad#cc#dd#...",
	"...####cc####...",
	"......####......"
};

KtopIconListElem::KtopIconListElem(const char* fName, const char* iName)
{
	pm = new QPixmap(fName);
	if (pm && pm->isNull())
	{
		delete pm;
		pm = NULL;
	}

	icnName = new char[strlen(iName) + 1];
	strcpy(icnName, iName);
}

QList<KtopIconListElem>* KtopIconList::icnList = NULL;
int KtopIconList::instCounter = 0;
const QPixmap* KtopIconList::defaultIcon = NULL;

KtopIconList::KtopIconList()
{
	/*
	 * Check whether the icons have already been loaded from another instance
	 * of this class. If not, load the icons.
	 */
	if (!KtopIconList::instCounter)
	{
		KtopIconList::load();
		defaultIcon = new QPixmap(execXpm);
		CHECK_PTR(defaultIcon);
	}

	// Increase the instance counter, we have created another instance.
	KtopIconList::instCounter++;
}

KtopIconList::~KtopIconList()
{ 
	// Decrease the instance counter.
	KtopIconList::instCounter--;

	/*
	 * If all instances have called the destructor we can delete the icon
	 * list.
	 */
	if (!KtopIconList::instCounter)
	{
		// delete default icon
		delete KtopIconList::defaultIcon;

		// delete other icons
		KtopIconList::icnList->setAutoDelete(TRUE);
		KtopIconList::icnList->clear();
	}
}

const QPixmap* 
KtopIconList::procIcon(const char* pname)
{
	KtopIconListElem* cur = KtopIconList::icnList->first();
	KtopIconListElem* last = KtopIconList::icnList->getLast();
	bool goOn = TRUE;

	/*
	 * Icons are matched to processes by name. The process name and the
	 * base name of the icon must be identical. This does not rule out some
	 * funny results but I can't think of a better way.
	 */
	QString iName = QString(pname) + ".xpm";

	// This linear search seems slow and complicated. I need to change this.
	do
	{
		if (cur == last)
			goOn = FALSE;
		if (!cur)
			goto end;
		if (strcmp(iName.data(), cur->name()) == 0)
		{
			const QPixmap *pix = cur->pixmap();
			if (pix)
				return (pix);
			else
				return (defaultIcon);
		}
		cur = KtopIconList::icnList->next(); 
	} while (goOn);

 end: 
	return defaultIcon;
}

void 
KtopIconList::load()
{
	DIR* dir;
	struct dirent* de;
	QString path;
	QString icnFile;

	icnList = new QList<KtopIconListElem>;
	CHECK_PTR(icnList);

	// determine path to mini icons
	path = Kapp->kde_icondir() + "/mini";
	dir = opendir(path);
  
	if (!dir)
		return;  // default icon will be used

    // Load all *.xpm files from the kde_icondir()/mini directory.
    while ((de = readdir(dir)))
	{
		if (strstr(de->d_name, ".xpm"))
		{ 
			// construct name of icon file
			icnFile = path + "/" + de->d_name;

			// read in icon
			KtopIconListElem *newElem = 
				new KtopIconListElem(icnFile, de->d_name);
			CHECK_PTR(newElem);

			// add icon to icon list
			icnList->append(newElem);  
		}       
	}

	closedir(dir);

	// deterime ktop data directory
	path = Kapp->kde_datadir() + "/ktop/pics";
    dir = opendir(path);     

    if (!dir)
		return; // default icon will be used
  
    // Load all *.xpm files from the kde_datadir()/ktop/pics directory.
	while ((de = readdir(dir)))
	{
		if (strstr(de->d_name, ".xpm")) 
		{ 
			// construct name of icon file
			icnFile = path + "/" + de->d_name;

			// read in icon
			KtopIconListElem *newElem =
				new KtopIconListElem(icnFile, de->d_name);
			CHECK_PTR(newElem);

			// add icon to icon list
			icnList->append(newElem);  
		}       
	}

	closedir(dir);
}

