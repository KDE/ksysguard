/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
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
#include <qwidget.h>
#include <qframe.h>
#include <qpicture.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpushbt.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qdialog.h>
#include <qchkbox.h>
#include <qlined.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <asm/param.h>
#include <sys/mman.h>

#include "settings.h"
#include "cpu.moc"
#include "kconfig.h"
#include "klocale.h"

/*=============================================================================
  GLOBALs
 =============================================================================*/
extern KConfig *config;

/*=============================================================================
 Class :  CpuMon (methods)
 =============================================================================*/
/*-----------------------------------------------------------------------------
  Routine : CpuMon::CpuMon (constructor)
 -----------------------------------------------------------------------------*/
CpuMon::CpuMon(QWidget *parent, const char *name, QWidget *child)
       :QWidget (parent, name)
{
    struct timezone timez;
    char dummy[10];
    int t;
    QString tmp;
    
    setBackgroundColor(black);
    setMinimumSize(200, 90);
    my_child = child;
    intervals = 100;  /* # of intervals we want to record in the load history */
    load_values = 0;
    load_values = (unsigned *)malloc(sizeof(unsigned) * intervals);
    memset(load_values, 0, sizeof(unsigned) * intervals);

    // set up the initial values
    statfile = 0;
    statfile = fopen("/proc/stat", "r");
    setvbuf(statfile, NULL, _IONBF, 0);
    
    // read in the initial tick values
    fscanf(statfile, "%s %d %d %d %d", dummy, &old_user_ticks
                                     , &old_nice_ticks, &old_system_ticks
                                     , &old_idle_ticks);
    gettimeofday(&oldtime, &timez);

    // get the configuration value for the update speed
    timer_interval = 2000; 
    
    tmp = config->readEntry(QString("PfmUpdate"));
    if(!tmp.isNull()) {
        if((t = tmp.toInt()))
            timer_interval = t * 1000;
    }
    
    tid = startTimer(timer_interval);
    iconified = FALSE;

    // set up 3 common needed brushes for the painting job
    brush_0 = QBrush(QColor("darkgreen"), SolidPattern);
    brush_1 = QBrush(QColor(   0, 225,   0), SolidPattern);
    brush_2 = QBrush(QColor( 230, 230,   0), SolidPattern);

    show();
}

/*-----------------------------------------------------------------------------
  Routine : CpuMon::~CpuMon (desstructor)
 -----------------------------------------------------------------------------*/
CpuMon::~CpuMon() 
{
  killTimer(tid);
  if ( load_values )
       free(load_values);
}

/*-----------------------------------------------------------------------------
  Routine : CpuMon::paintEvent
 -----------------------------------------------------------------------------*/
void CpuMon::paintEvent(QPaintEvent *)
{
    QPainter  p;
    unsigned  cur_y;
    char      buf[20];
    int       w = width() , 
              h = height();

    // first, paint the history display
    p.begin(this);
     p.setPen(QColor("darkgreen"));
     p.drawRect(0, 0, w, h);                 /* 3 horizontal scales */
     p.drawLine(0, h / 3, w, h / 3);
     p.drawLine(0, 2*(h / 3), w, 2*(h / 3));
     p.setPen(green);
     // set the viewport, so that we don't overwrite the frame
     p.setViewport(1, 1, w - 2, h - 2);
     // the coordinate translation.
     // x-axis uses # of intervals
     // y-axis uses 100 (100% = maximum load)
     p.setWindow(0, 0, intervals, 100);
     p.moveTo(0, h);
     for(int i = 0; i <(int) intervals; i++) {
         p.lineTo(i, 100 - load_values[i]);
     }
    p.end();

    w = my_child->width();
    h = my_child->height();

    // now paint the child (current load display)
    // this may be the iconified view, but my_child always contains the proper
    // widget pointer
    
    p.begin(my_child);
     p.setPen(green);
     p.setBackgroundMode(OpaqueMode);
     sprintf(buf, "%03d%%", load_values[intervals - 1]);
     p.drawText(1, h - 20, w - 2, 20, AlignCenter, buf);
     cur_y = load_values[intervals - 1];
     // again, set viewport to prevent the frame from beeing overwritten
     // the translation is simple: 100 units x-axis and 100 units (100%) y-axis
     p.setViewport(0, 10, w, h - 30);
     p.setWindow(0, 0, 100, 100);
     p.fillRect(20, 0, 60, 100 - cur_y, brush_0);
     p.fillRect(20, 100 - cur_y, 60, cur_y, brush_1);
    p.end();  
}

/*-----------------------------------------------------------------------------
  Routine : CpuMon::timerEvent : records the actual load values
 -----------------------------------------------------------------------------*/
void CpuMon::timerEvent(QTimerEvent *)
{
    char     buffer[100];
    struct   timeval time;
    struct   timezone timez;
    float    elapsed_time;
    unsigned elapsed_ticks, load_ticks, load_percent;

    // dirty trick :-)
    // shift the array one position up. memcpy() should work faster than
    // a simple for(;;) loop
    
    memcpy(load_values, &load_values[1], sizeof(unsigned) * (intervals - 1));

    rewind(statfile);
    fscanf(statfile, "%s %d %d %d %d", buffer, &user_ticks, &nice_ticks
                                     , &system_ticks, &idle_ticks );

    gettimeofday(&time, &timez);
    elapsed_time = (time.tv_sec - oldtime.tv_sec)
                 + (float) (time.tv_usec - oldtime.tv_usec) / 1000000.0;
    oldtime.tv_sec = time.tv_sec;
    oldtime.tv_usec = time.tv_usec;

    elapsed_ticks = (int)(elapsed_time * HZ * 100);
    load_ticks = (system_ticks - old_system_ticks) 
               + (nice_ticks - old_nice_ticks) 
               + (user_ticks - old_user_ticks);

    load_percent = (load_ticks * 100) / (elapsed_ticks / 100);

    if(load_percent > 100)
        load_percent = 100;

    load_values[intervals - 1] = load_percent;

    old_system_ticks = system_ticks;
    old_nice_ticks = nice_ticks;
    old_user_ticks = user_ticks;

    repaint();
}

/*-----------------------------------------------------------------------------
  Routine : CpuMon::setChild
 -----------------------------------------------------------------------------*/
void CpuMon::setChild(QWidget *w)
{
    my_child = w;
}

