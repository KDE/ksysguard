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
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "memory.moc"

/*=============================================================================
  #DEFINEs
 =============================================================================*/
#define bytetok(x) (((x) + 512) >> 10)
#define MEM_ZONE_SIZE 4096

/*=============================================================================
 Class : MemMon (methods)
 =============================================================================*/
/*-----------------------------------------------------------------------------
  Routine : MemMon::MemMon (constructor)
 -----------------------------------------------------------------------------*/
MemMon::MemMon(QWidget *parent, const char *name, QWidget *child)
       :QWidget (parent, name)
{
    char buffer[256];
    
    initMetaObject();

    setBackgroundColor(black);
    my_child = child;
    intervals = 100;
    mem_values = 0;
    mem_values = (int *)malloc(sizeof(int) * intervals);
    memset(mem_values, 0, sizeof(int) * intervals);
    FILE *fd = fopen("/proc/meminfo", "r");
    fgets(buffer, sizeof(buffer), fd);
    fgets(buffer, sizeof(buffer), fd);
    mem_size = physsize = atol(buffer + 5);
    fgets(buffer, sizeof(buffer), fd);
    mem_size += atol(buffer + 5);
    fclose(fd);
    brush_0 = QBrush(QColor("darkgreen"), SolidPattern);
    brush_1 = QBrush(green, SolidPattern);
    startTimer(2000);
    show();
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

    w = my_child->width();
    h = my_child->height();

    p.begin(my_child);
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
    char buffer[256];
    FILE *f;
    
    f = fopen("/proc/meminfo", "r");
    fgets(buffer, sizeof(buffer), f);
    fgets(buffer, sizeof(buffer), f);
    memcpy(mem_values, &mem_values[1], sizeof(int) * (intervals - 1));
    mem_values[intervals - 1] = atol(strchr(buffer + 6, ' ') + 1);
    fgets(buffer, sizeof(buffer), f);
    mem_values[intervals - 1] += atol(strchr(buffer + 6, ' ') + 1);
    fclose(f);
    if( isVisible() ) repaint();
}

/*=============================================================================
 Class : SwapMon (methods)
 =============================================================================*/
/*-----------------------------------------------------------------------------
  Routine : SwapMon::SwapMon
 -----------------------------------------------------------------------------*/
SwapMon::SwapMon(QWidget *parent, const char *name, QWidget *child )
        :QWidget (parent, name)
{
    initMetaObject();

    my_child  = child;
    ticks = 50;

    swapData = new unsigned [ticks](0);
    CHECK_PTR(swapData);
    memZone  = new char [4096](0);
    CHECK_PTR(memZone);

    setBackgroundColor(black);
    brush_0 = QBrush(QColor("darkgreen"), SolidPattern);
    brush_1 = QBrush(green, SolidPattern);
    brush_2 = QBrush(QColor("red"), SolidPattern);
    tid = startTimer(2000);
    timerEvent(NULL);
    show();
}

/*-----------------------------------------------------------------------------
  Routine : SwapMon::~SwapMon
 -----------------------------------------------------------------------------*/
SwapMon::~SwapMon()
{
  killTimer(tid);
  delete swapData; 
  delete memZone;
}

/*-----------------------------------------------------------------------------
  Routine : SwapMon::paintEvent
 -----------------------------------------------------------------------------*/
void SwapMon::paintEvent(QPaintEvent *)
{
  QPainter p;

  p.begin(this);
    p.setWindow(0, 0,ticks, 1000);
    p.setPen(red);
    p.moveTo(0,1000-swapData[(indx+1)%ticks]);
    for( unsigned count=1; count <= ticks ; count++ )
        p.lineTo(count,1000-swapData[(indx+count)%ticks]);

  p.end();

  p.begin(my_child);
        char buf[20];
	int w = my_child->width();
  	int h = my_child->height();
  	p.setPen(green);
  	p.setBackgroundMode(OpaqueMode);
  	sprintf(buf, "%5d K",swapVals[1]);
  	p.drawText(1, h - 20, w - 2, 20,AlignCenter,buf);
        p.setViewport(0, 10, w, h - 30);
        p.setWindow(0, 0, 100, 1000);
        p.fillRect(20, 0, 60, 1000, brush_0);
        p.fillRect(20, 1000-swapData[indx],60,swapData[indx],brush_2);
  p.end();
}

/*-----------------------------------------------------------------------------
  Routine : SwapMon::timerEvent
 -----------------------------------------------------------------------------*/
void SwapMon::timerEvent(QTimerEvent *)
{
  int   fd, len;
  char *p;
  
  /*
  fd = open("/proc/meminfo",O_RDONLY);
  if ( fd < 0 ) return;
  len = read(fd,memZone,MEM_ZONE_SIZE-1);
  close(fd);
  if ( ( len < 0 ) || (len > MEM_ZONE_SIZE-1) ) return;
  memZone[len] = '\0';

  // be prepared for extra columns to appear be seeking
  // to ends of lines...
  p = strchr(memZone,'\n');
  p = strchr(p+1,'\n');

  while (isspace(*p)) p++;
  while (*p && !isspace(*p)) p++;
	
  // "Swap total"	 
  swapVals[0] = bytetok(strtoul(p, &p, 10));
  swapVals[1] = bytetok(strtoul(p, &p, 10));
  // "Swap used"
  swapVals[2] = bytetok(strtoul(p, &p, 10));
  */

  if ( swapVals[0] == 0 ) swapVals[0] = unsigned(1e9);

  indx =(indx+1)%ticks;
  swapData[indx]=0; //(1000*swapVals[1])/swapVals[0]; @@@@@

  if( isVisible() ) repaint();


}




