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
#include <sys/time.h>

#ifdef __FreeBSD__
#include <kvm.h>
#include <nlist.h>
#endif

/*=============================================================================
  CLASSes
 =============================================================================*/
//-----------------------------------------------------------------------------
// class  : CpuMon 
//-----------------------------------------------------------------------------
class CpuMon : public QWidget
{

  Q_OBJECT

public:

   CpuMon (QWidget *parent = 0, const char *name = 0, QWidget *child = 0);
  ~CpuMon ();

   void updateValues();
   void setChild(QWidget *w = 0);
   int iconified;

protected:

  virtual void timerEvent(QTimerEvent *);
  virtual void paintEvent(QPaintEvent *);

  struct timeval oldtime;

  unsigned *load_values, 
            max_load;
  int       tid,
            old_y_scale;
  unsigned  intervals, 
	    timer_interval;
  unsigned  user_ticks, 
	    system_ticks, 
	    nice_ticks, 
	    idle_ticks,
            old_user_ticks, 
	    old_system_ticks, 
            old_nice_ticks, 
            old_idle_ticks;
  QWidget  *my_child;
  FILE     *statfile;
#ifdef __FreeBSD__
  u_long     cp_time_offset;
  u_long     HZ;

#define X_CCPU          0
#define X_CP_TIME       1
#define X_HZ            2
#define X_STATHZ        3
#define X_AVENRUN       4

  struct nlist nlst[X_AVENRUN + 2];

#endif
  QBrush    brush_0, 
            brush_1, 
            brush_2;
};

