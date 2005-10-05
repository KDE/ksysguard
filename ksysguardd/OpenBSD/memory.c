/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    Copyright (c) 1999-2000 Hans Petter Bieker <bieker@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/dkstat.h>
#include <sys/swap.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Command.h"
#include "memory.h"
#include "ksysguardd.h"

static size_t Total = 0;
static size_t MFree = 0;
static size_t Active = 0;
static size_t InActive = 0;
static size_t STotal = 0;
static size_t SFree = 0;
static size_t SUsed = 0;
static int pageshift = 0;

/* define pagetok in terms of pageshift */
#define pagetok(size) ((size) << pageshift)

void swapmode(int *used, int *total);

void
initMemory(struct SensorModul* sm)
{
	int pagesize;
	static int physmem_mib[] = { CTL_HW, HW_PHYSMEM };
	size_t size;
	/* get the page size with "getpagesize" and calculate pageshift from
	* it */
	pagesize = getpagesize();
	pageshift = 0;
	while (pagesize > 1) {
		pageshift++;
		pagesize >>= 1;
	}
	size = sizeof(Total);
	sysctl(physmem_mib, 2, &Total, &size, NULL, 0);
	Total /= 1024;
	swapmode(&SUsed, &STotal);

  registerMonitor("mem/physical/free", "integer", printMFree, printMFreeInfo, sm);
	registerMonitor("mem/physical/active", "integer", printActive, printActiveInfo, sm);
	registerMonitor("mem/physical/inactive", "integer", printInActive, printInActiveInfo, sm);
	registerMonitor("mem/swap/free", "integer", printSwapFree, printSwapFreeInfo, sm);
	registerMonitor("mem/swap/used", "integer", printSwapUsed, printSwapUsedInfo, sm);
}

void
exitMemory(void)
{
}

int
updateMemory(void)
{
	static int vmtotal_mib[] = {CTL_VM, VM_METER};
	size_t size;
	struct vmtotal vmtotal;
	size = sizeof(vmtotal);

	if (sysctl(vmtotal_mib, 2, &vmtotal, &size, NULL, 0) < 0)
		return -1;

	MFree = pagetok(vmtotal.t_free);
	MFree /= 1024;
	Active = pagetok(vmtotal.t_arm);
	Active /= 1024;
	InActive = pagetok(vmtotal.t_rm);
	InActive /= 1024;
	InActive -= Active;

	swapmode(&SUsed, &STotal);
	SFree = STotal - SUsed;
	return 0;
}

void
printMFree(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", MFree);
}

void
printMFreeInfo(const char* cmd)
{
	fprintf(CurrentClient, "Free Memory\t0\t%d\tKB\n", Total);
}

void
printActive(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", Active);
}

void
printActiveInfo(const char* cmd)
{
	fprintf(CurrentClient, "Active Memory\t0\t%d\tKB\n", Total);
}

void
printInActive(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", InActive);
}

void
printInActiveInfo(const char* cmd)
{
	fprintf(CurrentClient, "InActive Memory\t0\t%d\tKB\n", Total);
}

void
printSwapUsed(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", SUsed);
}

void
printSwapUsedInfo(const char* cmd)
{
	fprintf(CurrentClient, "Used Swap Memory\t0\t%d\tKB\n", STotal);
}

void
printSwapFree(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", SFree);
}

void
printSwapFreeInfo(const char* cmd)
{
	fprintf(CurrentClient, "Free Swap Memory\t0\t%d\tKB\n", STotal);
}

/*
This function swapmode was originally written by Tobias
Weingartner <weingart@openbsd.org>

Taken from OpenBSD top command
*/
void
swapmode (int *used, int *total)
{
	int     nswap, rnswap, i;
	struct swapent *swdev;

	*total = *used = 0;

	/* Number of swap devices */
	nswap = swapctl(SWAP_NSWAP, 0, 0);
	if (nswap == 0)
		return;

	swdev = (struct swapent *) malloc(nswap * sizeof(*swdev));
	if (swdev == NULL)
	 	return;

	rnswap = swapctl(SWAP_STATS, swdev, nswap);
	if (rnswap == -1) {
    free(swdev);
		return;
  }

	/* if rnswap != nswap, then what? */

	/* Total things up */
	for (i = 0; i < nswap; i++) {
		if (swdev[i].se_flags & SWF_ENABLE) {
			*used += (swdev[i].se_inuse / (1024 / DEV_BSIZE));
			*total += (swdev[i].se_nblks / (1024 / DEV_BSIZE));
		}
	}

	free(swdev);
}
