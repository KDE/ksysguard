/*
  KSysGuard, the KDE System Guard
   
  Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>
    
  This program is free software; you can redistribute it and/or
  modify it under the terms of version 2 of the GNU General Public
  License as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/socket.h>

#include <net/route.h>
#include <net/if.h>
#include <net/if_dl.h>

#include <ifaddrs.h>
#include <stdlib.h>
#include <string.h>

#include "Command.h"
#include "ksysguardd.h"
#include "netdev.h"


#define I_bytes		0
#define I_packs		1
#define I_errs		2
#define I_mcasts	3
#define I_lost		4

typedef struct {
  char name[32];
  u_long recv[5], Drecv[5], sent[5], Dsent[5];
} NetDevInfo;

#define LEN(X) (sizeof(X)/sizeof(X[0]))

#define MAXNETDEVS 64
static NetDevInfo NetDevs[MAXNETDEVS], newval[MAXNETDEVS];
static int NetDevCnt = 0;
static struct SensorModul *NetDevSM;

/* Read the system's traffic registers.
 * Merely count the IFs if countp nonzero.
 * Returns count of IFs read, or -1; the data are written into newval.
 * Based on getifaddrs source; getifaddrs itself seems to
 * compile incorrectly, omitting the traffic data. (It also
 * does things this doesn't need, thus this is slightly more efficient.)
 */
static int readSys(int countp) 
{
  size_t len;
  char *bfr, *ptr;
  struct rt_msghdr *rtm;
  NetDevInfo *nv;
  static int mib[] = {
    /* see sysctl(3): */
    CTL_NET,
    PF_ROUTE,
    0, /* `currently always 0' */
    0, /* `may be set to 0 to select all address families' */
    NET_RT_IFLIST,
    0 /* ignored but six levels are needed */
  };
     
  if (-1==sysctl(mib, LEN(mib), NULL, &len, NULL, 0))
    return -1;
  if (!(bfr = malloc(len)))
    return -1;
  if (-1==sysctl(mib, LEN(mib), bfr, &len, NULL, 0)) {
    free(bfr);
    return -1;
  }
  nv = newval;
  for (ptr=bfr; ptr<bfr+len; ptr+=rtm->rtm_msglen) {
    struct if_msghdr *ifm;
       
    rtm = (void*)ptr; /* chg ptr type to router msg */

    if (rtm->rtm_version != RTM_VERSION) {
      continue;
    }

    if (rtm->rtm_type != RTM_IFINFO) {
      continue;
    }

    ifm = (void*)rtm; /* chg ptr type to interface msg */
    if (!(ifm->ifm_flags & IFF_UP)) {
      continue;
    }
       
    if (!countp) {
      /* a sdl is concat'd to the if msg */
      struct sockaddr_dl *sdl = (void*)(ifm+1);
	 
      /* copy and terminate the name */
      /*fixme: check for overruns */
      memcpy(nv->name, sdl->sdl_data, sdl->sdl_nlen);
      nv->name[sdl->sdl_nlen] = 0;
	 
      /* copy the data */
      nv->recv[I_bytes]  = ifm->ifm_data.ifi_ibytes;
      nv->recv[I_packs]  = ifm->ifm_data.ifi_ipackets;
      nv->recv[I_errs]   = ifm->ifm_data.ifi_ierrors;
      nv->recv[I_mcasts] = ifm->ifm_data.ifi_imcasts;
      nv->recv[I_lost]   = ifm->ifm_data.ifi_iqdrops;
      nv->sent[I_bytes]  = ifm->ifm_data.ifi_obytes;
      nv->sent[I_packs]  = ifm->ifm_data.ifi_opackets;
      nv->sent[I_errs]   = ifm->ifm_data.ifi_oerrors;
      nv->sent[I_mcasts] = ifm->ifm_data.ifi_omcasts;
      nv->sent[I_lost]   = ifm->ifm_data.ifi_collisions;
    }
       
    /*fixme: guard against buffer overrun */
    nv++;
  }
  free(bfr);
  return nv-newval;
}


/* ------------------------------ public part --------------------------- */

static void prVal(const char*, int);
void printNetDevRecv(const char *cmd) { prVal(cmd,0); }  
void printNetDevSent(const char *cmd) { prVal(cmd,1); }
        
static void prInfo(const char*, int);
void printNetDevRecvInfo(const char *cmd) { prInfo(cmd,0); }
void printNetDevSentInfo(const char *cmd) { prInfo(cmd,1); }
        
static struct {
  char *label;
  cmdExecutor read, inform;
  struct {
    char *label, *info;
    int index;
  } op[5];
} opTable[] = {
  {"receiver",
   printNetDevRecv, printNetDevRecvInfo,
   {{"data", "Received Data\t0\t0\tB/s\n", I_bytes},
    {"packets", "Received Packets\t0\t0\tHz\n", I_packs},
    {"errors", "Receiver Errors\t0\t0\tHz\n", I_errs},
    {"multicast", "Received Multicast Packets\t0\t0\tHz\n", I_mcasts},
    {"drops", "Receiver Drops\t0\t0\tHz\n", I_lost}}},
  {"transmitter",
   printNetDevSent, printNetDevSentInfo,
   {{"data", "Sent Data\t0\t0\tB/s\n", I_bytes},
    {"packets", "Sent Packets\t0\t0\tHz\n", I_packs},
    {"errors", "Transmitter Errors\t0\t0\tHz\n", I_errs},
    {"multicast", "Sent Multicast Packets\t0\t0\tHz\n", I_mcasts},
    {"collisions", "Transmitter Collisions\t0\t0\tHz\n", I_lost}}}
};


static void prVal(const char *cmd, int N) {
  char *p, *q, *r;
  int i, d;

  if (!(p=rindex(cmd, '/')))
    return;
  *p=0;
  q=rindex(cmd, '/');
  *q=0;
  r=rindex(cmd, '/');
  r++;
  for (d=NetDevCnt; d--; )
    if (!strcmp(r, NetDevs[d].name))
      break;
  *q=*p='/';
  
  if (-1 == d) return;
  
  p++;
  for (i=0; i<LEN(opTable[0].op); i++)
    if (!strcmp(p, opTable[N].op[i].label))
      fprintf(CurrentClient, "%lu",
	      /*fixme: ugly and presumptuous */
	      (N?NetDevs[d].Dsent:NetDevs[d].Drecv)[opTable[N].op[i].index]);
  fprintf(CurrentClient, "\n");
}


static void prInfo(const char *cmd, int N) {
  char *p, *q;
  int i;
  
  if (!(p=rindex(cmd, '/'))) return;
  p++;

  q = p+strlen(p)-1;
  if ('?' != *q) return;
  *q=0;

  for (i=0; i<LEN(opTable[0].op); i++)
    if (!strcmp(p, opTable[N].op[i].label))
      fputs(opTable[N].op[i].info, CurrentClient);
  *q='?';
}



static void NDreg (int setp)
{
  int i;
	
  for (i = 0; i<NetDevCnt; i++) {
    int j;

    for (j=0; j<LEN(opTable); j++) {
      int k;

      for (k=0; k<LEN(opTable[0].op); k++) {
	char buffer[1024];

	snprintf(buffer, sizeof(buffer),
		 "network/interfaces/%s/%s/%s",
		 NetDevs[i].name,
		 opTable[j].label,
		 opTable[j].op[k].label);

	/* printf("%d %d %d %s\n",i,j,k,buffer); */

	if (setp)
	  registerMonitor(buffer,
			  "integer",
			  opTable[j].read,
			  opTable[j].inform, NetDevSM);
	else
	  removeMonitor(buffer);
      }

    }
  }
}

void initNetDev(struct SensorModul* sm) {
  int i;

  NetDevSM = sm;
 
  updateNetDev();

  for (i=LEN(NetDevs); i--;) {
    strcpy(NetDevs[i].name, newval[i].name);
  }

  NDreg(!0);
}


void exitNetDev(void) {
  NDreg(0);
}

void updateNetDev(void) {
  NetDevInfo *p, *q;
  int n;

  if (-1==(n = readSys(0)))
    return;

  NetDevCnt = n;
  /*fixme: assumes the interfaces are in the same order each time */
  for (p=NetDevs, q=newval; n--; p++, q++) {
    int i;
    /* calculate deltas */
    for (i=0; i<5; i++) {
      p->Drecv[i] = q->recv[i]-p->recv[i];
      p->recv[i]  = q->recv[i];
      p->Dsent[i] = q->sent[i]-p->sent[i];
      p->sent[i]  = q->sent[i];

    }
  }
}

void checkNetDev(void) {
  if (readSys(!0) != NetDevCnt) {
    /* interface has been added or removed
       so we do a reset */
    exitNetDev();
    initNetDev(NetDevSM);
  }
}


/* eof */
