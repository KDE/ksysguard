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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#ifdef __FreeBSD__
#include <sys/sysctl.h>
#endif

#include "memory.moc"

/*=============================================================================
 Class : MemMon (methods)
 =============================================================================*/
/*-----------------------------------------------------------------------------
  Routine : MemMon::MemMon (constructor)
 -----------------------------------------------------------------------------*/
MemMon::MemMon(QWidget *parent, const char *name, QWidget *child)
       :QWidget (parent, name)
{
#ifdef __FreeBSD__
    int mib[2],memory;size_t len;
#else
    char buffer[256];
#endif
    
    setBackgroundColor(black);
    my_child = child;
    intervals = 100;
    mem_values = 0;
    mem_values = (int *)malloc(sizeof(int) * intervals);
    memset(mem_values, 0, sizeof(int) * intervals);
#ifdef __FreeBSD__
    // incomplete
#warning mem_size was set to some random value
    mib[0]=CTL_HW;mib[1]=HW_PHYSMEM;
    len=sizeof(memory);
    sysctl(mib,2,&physsize,&len,NULL,0);
    mem_size = physsize + 50;
#else
    FILE *fd = fopen("/proc/meminfo", "r");
    fgets(buffer, sizeof(buffer), fd);
    fgets(buffer, sizeof(buffer), fd);
    mem_size = physsize = atol(buffer + 5);
    fgets(buffer, sizeof(buffer), fd);
    mem_size += atol(buffer + 5);
    fclose(fd);
#endif
    brush_0 = QBrush(QColor("darkgreen"), SolidPattern);
    brush_1 = QBrush(green, SolidPattern);
    startTimer(2000);
}

/*-----------------------------------------------------------------------------
  Routine : MemMon::MemMon (destructor)
 -----------------------------------------------------------------------------*/
MemMon::~MemMon() 
{
    if(mem_values)
        free(mem_values);
}

/*-----------------------------------------------------------------------------
  Routine : MemMon::paintEvent
 -----------------------------------------------------------------------------*/
void MemMon::paintEvent(QPaintEvent *)
{
    QPainter p;
    char     buf[20];
    int      one_percent = mem_size / 100, cur;
    int      w, h;
    
    p.begin(this);
     p.setPen(red);
     p.setWindow(0, 0, intervals, 100);
     int sep = physsize / one_percent;
     p.drawLine(0, 100 - sep, intervals, 100 - sep);
     p.setPen(QColor("darkgreen"));
     p.drawRect(0, 0, width(), height());
     p.setPen(green);
     p.moveTo(0, 100);
     for(int i = 0; i < intervals; i++) {
       p.lineTo(i, 100 - mem_values[i] / one_percent);
     }
    p.end();

    p.begin(my_child);
     w = my_child->width();
     h = my_child->height();
     cur = mem_values[intervals - 1] / one_percent;
     p.setPen(green);
     p.setBackgroundMode(OpaqueMode);
     sprintf(buf, "%5d K", mem_values[intervals - 1] / 1024);
     p.drawText(1, h - 20, w - 2, 20, AlignCenter, buf);
     p.setViewport(0, 10, w, h - 30);
     p.setWindow(0, 0, 100, 100);
     p.fillRect(20, 0, 60, 100 - cur, brush_0);
     p.fillRect(20, 100 - cur, 60, cur, brush_1);
    p.end();
}

/*-----------------------------------------------------------------------------
  Routine : MemMon::timerEvent
 -----------------------------------------------------------------------------*/
void MemMon::timerEvent(QTimerEvent *)
{
#ifdef __FreeBSD__
    // doesn't for on BSD yet
#else
    char buffer[256];
    FILE *f;
    
    f = fopen("/proc/meminfo", "r");
    fgets(buffer, sizeof(buffer), f);
    fgets(buffer, sizeof(buffer), f);
#endif
    memcpy(mem_values, &mem_values[1], sizeof(int) * (intervals - 1));
#ifdef __FreeBSD__
    mem_values[intervals - 1] = 4;
    mem_values[intervals - 1] += 3;
#else
    mem_values[intervals - 1] = atol(strchr(buffer + 6, ' ') + 1);
    fgets(buffer, sizeof(buffer), f);
    mem_values[intervals - 1] += atol(strchr(buffer + 6, ' ') + 1);
    fclose(f);
#endif
    if( isVisible() ) repaint();
}

