/*
    $Id$

    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu
    
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

/*=============================================================================
  HEADERs
 =============================================================================*/
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

/*=============================================================================
  #DEFINEs
 =============================================================================*/
#define ktr  klocale->translate

/*=============================================================================
  GLOBALs
 =============================================================================*/
extern KConfig *config;


/*=============================================================================
 Class : AppSettings (methods)
 =============================================================================*/
/*-----------------------------------------------------------------------------
  Routine : AppSettings::AppSettings (constructor)
 -----------------------------------------------------------------------------*/
AppSettings::AppSettings(QWidget *parent, const char *name)
            :QDialog(parent, name, TRUE)
{
    setMinimumSize(250,170);
    setMaximumSize(250,170);
    setCaption(i18n("Ktop Preferences"));
    startuppage = new QButtonGroup(i18n("Start up page"), this, "_startuppage");
    rb_pList = new QRadioButton(i18n("Processes List"),startuppage, "_pList");
    rb_pTree = new QRadioButton(i18n("Processes Tree"),startuppage, "_pTree");
    rb_Perf  = new QRadioButton(i18n("Performance"),startuppage, "_Perf");
    startuppage->resize(300, 200);

    ok = new QPushButton(i18n("OK"), this, "_ok");
    ok->setDefault(TRUE);
    cancel = new QPushButton(i18n("Cancel"), this, "_cancel");
    ok->resize(80, 25);
    cancel->resize(80, 25);
    connect(ok, SIGNAL(clicked()), this, SLOT(doValidate()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    adjustSize();
}

/*-----------------------------------------------------------------------------
  Routine : AppSettings::AppSettings (destructor)
 -----------------------------------------------------------------------------*/
AppSettings::~AppSettings()
{
}

/*-----------------------------------------------------------------------------
  Routine : AppSettings::resizeEvent
 -----------------------------------------------------------------------------*/
void AppSettings::resizeEvent(QResizeEvent *)
{
    startuppage->setGeometry(10, 10, width()-20, height() - 65);
    ok->move(16, height() - 38);
    cancel->move(16+ok->width()+16, height() - 38);
    rb_pList->move(20,17);
    rb_pTree->move(20,42);
    rb_Perf->move(20,67);
}

/*-----------------------------------------------------------------------------
  Routine : AppSettings::setStartUpPage
 -----------------------------------------------------------------------------*/
void AppSettings::setStartUpPage(int update)
{
    switch(update) {
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

/*-----------------------------------------------------------------------------
  Routine : AppSettings::getStartUpPage
 -----------------------------------------------------------------------------*/
int AppSettings::getStartUpPage()
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

/*-----------------------------------------------------------------------------
  Routine : AppSettings::doValidate
 -----------------------------------------------------------------------------*/
void AppSettings::doValidate()
{
    accept();
}


