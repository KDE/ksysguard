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
#include <qlabel.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#ifdef __FreeBSD__
#include <sys/sysctl.h>
#include <sys/rlist.h>
#include <sys/conf.h>
#include <vm/vm_param.h>
#include <osreldate.h>        // important, catch kernel changes 2.2.x > 3.x.x
#endif

#include "memory.moc"

#ifdef __FreeBSD__
extern kvm_t *kvm;
static int pageshift;
#define pagetok(size) ((size) << pageshift)       // translate pages to K(bytes)
#define __DEBUG__
#endif

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
    int mib[2];
    size_t len;
    int pagesize;
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

    // get physical pagesize, calculate pageshift value for pagetok()
    // taken from FreeBSD's part of top

    iVm = &ivm;
    pagesize = getpagesize();
    pageshift = 0;
    while (pagesize > 1)
    {
        pageshift++;
        pagesize >>= 1;
    }

    //    pageshift -= LOG1024;
    kvm_nlist(kvm, nlst);
    cnt_offset = nlst[X_CNT].n_value;
    buf_offset = nlst[X_BUFSPACE].n_value;

    // init vmmeter the first time
    kvm_read(kvm, cnt_offset, &vmstat, sizeof(vmstat));

    // bufspace is for "buffers", this is not in vmmeter
    kvm_read(kvm, buf_offset, &bufspace, sizeof(bufspace));

    mib[0] = CTL_HW;
    mib[1] = HW_PHYSMEM;

    len = sizeof(physmem);
    sysctl(mib, 2, &physmem, &len, NULL, 0);
    mem_size = physmem;

    // get available pyhsical ram for userspace
    mib_usermem[0] = CTL_HW;
    mib_usermem[1] = HW_USERMEM;
    sysctl(mib_usermem, 2, &usermem, &len, NULL, 0);

    int tempsw = swapInfo(&sw_avail, &sw_free);

    mem_size = physmem / 1024 + sw_avail;

#ifdef __DEBUG__
    fprintf(stdout, "TotalVirtual: %d K\n", mem_size);
    fprintf(stdout, "Swap avail: %d, free: %d\n", sw_avail, sw_free);
#endif

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

// obtain information about available swapspace
// this is taken from the machine - dependent part of FreeBSDs top
// implementation.
// here is the original header of this file (machine.c)

/* top - a top users display for Unix
 *
 * SYNOPSIS:  For FreeBSD-2.x system
 *
 * DESCRIPTION:
 * Originally written for BSD4.4 system by Christos Zoulas.
 * Ported to FreeBSD 2.x by Steven Wallace && Wolfram Schneider
 * Order support hacked in from top-3.5beta6/machine/m_aix41.c
 *   by Monte Mitzelfelt (for latest top see http://www.groupsys.com/topinfo/)
 *
 * This is the machine-dependent module for FreeBSD 2.2
 * Works for:
 *	FreeBSD 2.2, and probably FreeBSD 2.1.x
 *
 * LIBS: -lkvm
 *
 * AUTHOR:  Christos Zoulas <christos@ee.cornell.edu>
 *          Steven Wallace  <swallace@freebsd.org>
 *          Wolfram Schneider <wosch@FreeBSD.org>
 */

#ifdef __FreeBSD__

/*
 * swapmode is based on a program called swapinfo written
 * by Kevin Lahey <kml@rokkaku.atl.ga.us>.
 */

#define	SVAR(var) __STRING(var)	/* to force expansion */
#define	KGET(idx, var)							\
	KGET1(idx, &var, sizeof(var), SVAR(var))
#define	KGET1(idx, p, s, msg)						\
	KGET2(nlst[idx].n_value, p, s, msg)
#define	KGET2(addr, p, s, msg)						\
	if (kvm_read(kvm, (u_long)(addr), p, s) != s) {		        \
		return (0);                                             \
    }
//		warnx("cannot read %s: %s", msg, kvm_geterr(kvm));       \

#define	KGETRET(addr, p, s, msg)					\
	if (kvm_read(kvm, (u_long)(addr), p, s) != s) {			\
		return (0);						\
	}
//		warnx("cannot read %s: %s", msg, kvm_geterr(kvm));	\


int MemMon::swapInfo(int *retavail, int *retfree)
{
	char *header;
	int hlen, nswap, nswdev, dmmax;
	int i, div, avail, nfree, npfree, used;
	struct swdevt *sw = 0;
	long blocksize, *perdev = 0;
	u_long ptr;
	struct rlist head;
#if __FreeBSD_version >= 220000
	struct rlisthdr swaplist;
#else 
	struct rlist *swaplist = 0;
#endif
	struct rlist *swapptr = 0;

	/*
	 * Counter for error messages. If we reach the limit,
	 * stop reading information from swap devices and
	 * return zero. This prevent endless 'bad address'
	 * messages.
	 */
	static warning = 10;

	if (warning <= 0) {
	    /* a single warning */
	    if (!warning) {
		warning--;
		fprintf(stderr, 
			"Too much errors, stop reading swap devices ...\n");
		(void)sleep(3);
	    }
	    return(0);
	}
	warning--; /* decrease counter, see end of function */

	KGET(VM_NSWAP, nswap);
	if (!nswap) {
		fprintf(stderr, "No swap space available\n");
		return(0);
	}

	KGET(VM_NSWDEV, nswdev);
	KGET(VM_DMMAX, dmmax);
	KGET1(VM_SWAPLIST, &swaplist, sizeof(swaplist), "swaplist");
        if ((sw = (struct swdevt *)malloc(nswdev * sizeof(*sw))) == NULL ||
            (perdev = (long *)malloc(nswdev * sizeof(*perdev))) == NULL)
            fprintf(stderr, "Fatal in MemMon::swapInfo - malloc failed\n");
	KGET1(VM_SWDEVT, &ptr, sizeof(ptr), "swdevt");
	KGET2(ptr, sw, nswdev * sizeof(*sw), "*swdevt");

	/* Count up swap space. */
	nfree = 0;
	memset(perdev, 0, nswdev * sizeof(*perdev));
#if  __FreeBSD_version >= 220000
	swapptr = swaplist.rlh_list;
	while (swapptr)
#else
	while (swaplist)
#endif
        {
		int	top, bottom, next_block;
#if  __FreeBSD_version >= 220000
		KGET2(swapptr, &head, sizeof(struct rlist), "swapptr");
#else
		KGET2(swaplist, &head, sizeof(struct rlist), "swaplist");
#endif

		top = head.rl_end;
		bottom = head.rl_start;

		nfree += top - bottom + 1;

		/*
		 * Swap space is split up among the configured disks.
		 *
		 * For interleaved swap devices, the first dmmax blocks
		 * of swap space some from the first disk, the next dmmax
		 * blocks from the next, and so on up to nswap blocks.
		 *
		 * The list of free space joins adjacent free blocks,
		 * ignoring device boundries.  If we want to keep track
		 * of this information per device, we'll just have to
		 * extract it ourselves.
		 */
		while (top / dmmax != bottom / dmmax) {
			next_block = ((bottom + dmmax) / dmmax);
			perdev[(bottom / dmmax) % nswdev] +=
				next_block * dmmax - bottom;
			bottom = next_block * dmmax;
		}
		perdev[(bottom / dmmax) % nswdev] +=
			top - bottom + 1;

#if  __FreeBSD_version >= 220000
		swapptr = head.rl_next;
#else
		swaplist = head.rl_next;
#endif
	}

	header = getbsize(&hlen, &blocksize);
	div = blocksize / 512;
	avail = npfree = 0;
	for (i = 0; i < nswdev; i++) {
		int xsize, xfree;

		/*
		 * Don't report statistics for partitions which have not
		 * yet been activated via swapon(8).
		 */

		xsize = sw[i].sw_nblks;
		xfree = perdev[i];
		used = xsize - xfree;
		npfree++;
		avail += xsize;
	}

	/* 
	 * If only one partition has been set up via swapon(8), we don't
	 * need to bother with totals.
	 */
	*retavail = avail / 2;
	*retfree = nfree / 2;
	used = avail - nfree;
	free(sw); free(perdev);

	/* increase counter, no errors occurs */
	warning++; 

        //	return  (int)(((double)used / (double)avail * 100.0) + 0.5);
        return used;
}
#endif


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

    u_long usermem;
    int len = sizeof(usermem);
    
    kvm_read(kvm, cnt_offset, &vmstat, sizeof(vmstat));
    kvm_read(kvm, buf_offset, &bufspace, sizeof(bufspace));
    sysctl(mib_usermem, 2, &usermem, &len, NULL, 0);

    iVm->active = pagetok(vmstat.v_active_count) / 1024;
    iVm->inactive = pagetok(vmstat.v_inactive_count) / 1024;
    iVm->wired = pagetok(vmstat.v_wire_count) / 1024;
    iVm->cache = pagetok(vmstat.v_cache_count) / 1024;
    iVm->buffers = bufspace / 1024;
    
    updateLabel(memstat_labels[0], iVm->active);
    updateLabel(memstat_labels[1], iVm->inactive);
    updateLabel(memstat_labels[2], iVm->wired);
    updateLabel(memstat_labels[3], iVm->cache);
    updateLabel(memstat_labels[MEM_BUFFERS], iVm->buffers);
    updateLabel(memstat_labels[MEM_PHYS], physmem / 1024);
    updateLabel(memstat_labels[MEM_KERN], (physmem - usermem) / 1024);
    updateLabel(memstat_labels[MEM_SWAPAVAIL], sw_avail);
    updateLabel(memstat_labels[MEM_SWAPFREE], sw_free);
    updateLabel(memstat_labels[MEM_TOTAL], mem_size);
#else
    char buffer[256];
    FILE *f;
    
    f = fopen("/proc/meminfo", "r");
    fgets(buffer, sizeof(buffer), f);
    fgets(buffer, sizeof(buffer), f);
#endif
    memcpy(mem_values, &mem_values[1], sizeof(int) * (intervals - 1));
#ifdef __FreeBSD__
    mem_values[intervals - 1] = iVm->active + iVm->inactive + iVm->wired + iVm->cache + iVm->buffers;
#else
    mem_values[intervals - 1] = atol(strchr(buffer + 6, ' ') + 1);
    fgets(buffer, sizeof(buffer), f);
    mem_values[intervals - 1] += atol(strchr(buffer + 6, ' ') + 1);
    fclose(f);
#endif
    if( isVisible() ) repaint();
}

#ifdef __FreeBSD__
void MemMon::setTargetLabels(QLabel **labels)
{
    QLabel *temp;
    memstat_labels = labels;
}
#endif

// update one of the memory statistics labels.

void MemMon::updateLabel(QLabel *l, int value)
{
    char temp[100];

    snprintf(temp, 90, "%8d K", value);
    l->setText(temp);
}
