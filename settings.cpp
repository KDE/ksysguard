/*
    $Id$

    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu
    
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

#include <qpainter.h>
#include <qpushbt.h>
#include <qtabdlg.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qobject.h>
#include <qlistbox.h>
#include <qgrpbox.h>
#include <qradiobt.h>
#include <qchkbox.h>
#include <qbttngrp.h>
#include <qpalette.h>
#include <qlined.h>
#include <qmsgbox.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <time.h>

#include <kconfig.h>
#include <klocale.h>
#include <kapp.h>

#include "settings.moc"
#include "cpu.h"
#include "memory.h"
#include "widgets.h"

AppSettings::AppSettings(QWidget *parent, const char *name)
            :QDialog(parent, name, TRUE)
{
    setMinimumSize(210,170);
    setMaximumSize(210,170);
    setCaption(i18n("Ktop Preferences"));

    startuppage = new QButtonGroup(i18n("Start up page"), this,
								   "_startuppage");

    rb_pList = new QRadioButton(i18n("Processes List"),startuppage, "_pList");
	rb_pList->resize(160, 25);
    rb_pTree = new QRadioButton(i18n("Processes Tree"),startuppage, "_pTree");
	rb_pTree->resize(160, 25);
    rb_Perf  = new QRadioButton(i18n("Performance Meter"),startuppage, "_Perf");
	rb_Perf->resize(160, 25);

    ok = new QPushButton(i18n("Ok"), this, "_ok");
    ok->setDefault(TRUE);
    cancel = new QPushButton(i18n("Cancel"), this, "_cancel");
    ok->resize(80, 25);

    cancel->resize(80, 25);
    connect(ok, SIGNAL(clicked()), this, SLOT(doValidate()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void 
AppSettings::resizeEvent(QResizeEvent *)
{
	startuppage->setGeometry(10, 10, width() - 20, height() - 65);
	ok->move(16, height() - 38);
	cancel->move(16+ok->width() + 16, height() - 38);
	rb_pList->move(20,17);
	rb_pTree->move(20,42);
	rb_Perf->move(20,67);
}

void 
AppSettings::setStartUpPage(int update)
{
    switch(update)
	{
	case TaskMan::PAGE_PLIST:
	case TaskMan::PAGE_PTREE:
	case TaskMan::PAGE_PERF:
		rb_pList->setChecked(update == TaskMan::PAGE_PLIST);
		rb_pTree->setChecked(update == TaskMan::PAGE_PTREE);
		rb_Perf->setChecked (update == TaskMan::PAGE_PERF );
		break;
	default:
		rb_pList->setChecked(TRUE);
		break;
	}
}

int 
AppSettings::getStartUpPage()
{
    int page = 0;
    
    if ( rb_pList->isChecked() )
        page = TaskMan::PAGE_PLIST;
    if ( rb_pTree->isChecked() )
        page = TaskMan::PAGE_PTREE;
    if ( rb_Perf->isChecked()  )
        page = TaskMan::PAGE_PERF;

    return page;
}

void 
AppSettings::doValidate()
{
    accept();
}


