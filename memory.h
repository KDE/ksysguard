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

#ifdef __FreeBSD__
#include <sys/vmmeter.h>
#include <kvm.h>
#include <nlist.h>
#endif

/*=============================================================================
  CLASSes
 =============================================================================*/
//-----------------------------------------------------------------------------
// class  : MemMon
//-----------------------------------------------------------------------------
class MemMon : public QWidget
{
    Q_OBJECT

public:     
     MemMon (QWidget *parent = 0, const char *name = 0, QWidget *child = 0);
    ~MemMon ();

#ifdef __FreeBSD__
     void setTargetLabels(QLabel **);
     int  swapInfo(int *, int *);
#endif
     void updateLabel(QLabel *, int);
protected:

    virtual void paintEvent(QPaintEvent *);
    virtual void timerEvent(QTimerEvent *);

    int      intervals,
             mem_size, 
             physsize,
            *mem_values;
    QWidget *my_child;
    QBrush   brush_0, 
             brush_1;
#ifdef __FreeBSD__
    u_long physmem;
    int    sw_avail, sw_free;
    int    bufspace;
    QLabel **memstat_labels;
    struct vmmeter vmstat;
    u_long   cnt_offset, buf_offset;

#define X_CNT          0
#define X_VMTOTAL      1
#define X_BUFSPACE     2
#define VM_SWAPLIST     3
#define VM_SWDEVT       4
#define VM_NSWAP        5
#define VM_NSWDEV       6
#define VM_DMMAX        7

    struct nlist nlst[VM_DMMAX + 2];

    struct _ivm {
        int active, inactive, wired, cache, buffers, unused, usermem;
    };
    struct _ivm ivm, *iVm;
#endif
};

#define MEM_ACTIVE 0
#define MEM_INACTIVE 1
#define MEM_WIRED 2
#define MEM_CACHE 3
#define MEM_BUFFERS 4
#define MEM_PHYS 5
#define MEM_UNUSED 6
#define MEM_SWAPAVAIL 7
#define MEM_SWAPFREE  8
#define MEM_TOTAL 9
#define MEM_END 10
