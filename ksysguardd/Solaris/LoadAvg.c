/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

	Solaris support by Torsten Kasch <tk@Genetik.Uni-Bielefeld.DE>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef HAVE_KSTAT
#include <kstat.h>
#endif

#include "ksysguardd.h"
#include "Command.h"
#include "LoadAvg.h"

#define MAX_CPU_COUNT 512
double loadavg1 = 0.0;
double loadavg5 = 0.0;
double loadavg15 = 0.0;

typedef struct {
	int32_t		ks_instance;
	uint64_t	cpu_nsecs_idle;
	uint64_t	cpu_nsecs_kernel;
	uint64_t	cpu_nsecs_user;
	uint64_t	cpu_nsecs_wait;
	float		idle_load;
	float		kernel_load;
	float		user_load;
	float		wait_load;
} cpu_loadinfo;

static cpu_loadinfo cpu_load[MAX_CPU_COUNT];

void initLoadAvg( struct SensorModul* sm ) {
	long nproc, id;

#ifdef HAVE_KSTAT
	registerMonitor( "cpu/system/loadavg1", "float", printLoadAvg1, printLoadAvg1Info, sm );
	registerMonitor( "cpu/system/loadavg5", "float", printLoadAvg5, printLoadAvg5Info, sm );
	registerMonitor( "cpu/system/loadavg15", "float", printLoadAvg15, printLoadAvg15Info, sm );
	
	/* Monitor names changed from kde3 => kde4. Remain compatible with legacy requests when possible. */
	registerLegacyMonitor( "cpu/loadavg1", "float", printLoadAvg1, printLoadAvg1Info, sm );
	registerLegacyMonitor( "cpu/loadavg5", "float", printLoadAvg5, printLoadAvg5Info, sm );
	registerLegacyMonitor( "cpu/loadavg15", "float", printLoadAvg15, printLoadAvg15Info, sm );

	for (id=0; id<MAX_CPU_COUNT; id++) {
		cpu_load[id].ks_instance = -1;
		cpu_load[id].cpu_nsecs_idle = 0;
		cpu_load[id].cpu_nsecs_kernel = 0;
		cpu_load[id].cpu_nsecs_user = 0;
		cpu_load[id].cpu_nsecs_wait = 0;
		cpu_load[id].idle_load = 0;
		cpu_load[id].kernel_load = 0;
		cpu_load[id].user_load = 0;
		cpu_load[id].wait_load = 0;
	}

	nproc = sysconf(_SC_NPROCESSORS_ONLN);
	for (id=0; id<nproc; id++) {
		char cmdName[ 24 ];
		sprintf( cmdName, "cpu/cpu%d/user", id );
		registerMonitor( cmdName, "float", printCPUxUser, printCPUxUserInfo, sm );
		sprintf( cmdName, "cpu/cpu%d/sys", id );
		registerMonitor( cmdName, "float", printCPUxKernel, printCPUxKernelInfo, sm );
		sprintf( cmdName, "cpu/cpu%d/TotalLoad", id );
		registerMonitor( cmdName, "float", printCPUxTotalLoad, printCPUxTotalLoadInfo, sm );
		sprintf( cmdName, "cpu/cpu%d/idle", id );
		registerMonitor( cmdName, "float", printCPUxIdle, printCPUxIdleInfo, sm );
		sprintf( cmdName, "cpu/cpu%d/wait", id );
		registerMonitor( cmdName, "float", printCPUxWait, printCPUxWaitInfo, sm );
	}
	updateLoadAvg();
#endif
}

void exitLoadAvg( void ) {
#ifdef HAVE_KSTAT
	removeMonitor("cpu/system/loadavg1");
	removeMonitor("cpu/system/loadavg5");
	removeMonitor("cpu/system/loadavg15");
	
	/* These were registered as legacy monitors */
	removeMonitor("cpu/loadavg1");
	removeMonitor("cpu/loadavg5");
	removeMonitor("cpu/loadavg15");
#endif
}

int updateLoadAvg( void ) {

#ifdef HAVE_KSTAT
	kstat_ctl_t		*kctl;
	kstat_t			*ksp;
	kstat_named_t		*kdata;
	int i;

	/*
	 *  get a kstat handle and update the user's kstat chain
	 */
	if( (kctl = kstat_open()) == NULL )
		return( 0 );
	while( kstat_chain_update( kctl ) != 0 )
		;

	for (i=0; i<MAX_CPU_COUNT && cpu_load[i].ks_instance != -1; i++) {
		cpu_load[i].idle_load = 0;
		cpu_load[i].kernel_load = 0;
		cpu_load[i].user_load = 0;
		cpu_load[i].wait_load = 0;
	}

	/*
	 *  traverse the kstat chain to find the appropriate statistics
	 */
	/* if( (ksp = kstat_lookup( kctl, "unix", 0, "system_misc" )) == NULL )
		return( 0 );*/
	for (ksp = kctl->kc_chain; ksp; ksp = ksp->ks_next) {
		if (strcmp(ksp->ks_module, "unix") == 0 &&
		    strcmp(ksp->ks_name, "system_misc") == 0) {

			if( kstat_read( kctl, ksp, NULL ) == -1 )
				continue;

			/*
			 *  lookup the data
			 */
			 kdata = (kstat_named_t *) kstat_data_lookup( ksp, "avenrun_1min" );
			 if( kdata != NULL )
				loadavg1 = LOAD( kdata->value.ui32 );
			 kdata = (kstat_named_t *) kstat_data_lookup( ksp, "avenrun_5min" );
			 if( kdata != NULL )
				loadavg5 = LOAD( kdata->value.ui32 );
			 kdata = (kstat_named_t *) kstat_data_lookup( ksp, "avenrun_15min" );
			 if( kdata != NULL )
				loadavg15 = LOAD( kdata->value.ui32 );

		} else if (strcmp(ksp->ks_module, "cpu") == 0 &&
		    strcmp(ksp->ks_name, "sys") == 0) {
			int found;

			found = 0;
			for (i=0; i<MAX_CPU_COUNT; i++) {
				if (cpu_load[i].ks_instance == ksp->ks_instance) {
					found = 1;
					break;
				}
				if (cpu_load[i].ks_instance == -1) {
					cpu_load[i].ks_instance = ksp->ks_instance;
					found = 2;
					break;
				}
			}
			if (found) {
				uint64_t curr_cpu_nsecs_idle, curr_cpu_nsecs_kernel;
				uint64_t curr_cpu_nsecs_user, curr_cpu_nsecs_wait;
				double totalNsecs;

				if( kstat_read( kctl, ksp, NULL ) == -1 )
					continue;

				kdata = (kstat_named_t *) kstat_data_lookup( ksp, "cpu_nsec_idle" );
				if (kdata != NULL)
					curr_cpu_nsecs_idle = LOAD( kdata->value.ui64 );
				kdata = (kstat_named_t *) kstat_data_lookup( ksp, "cpu_nsec_kernel" );
				if (kdata != NULL)
					curr_cpu_nsecs_kernel = LOAD( kdata->value.ui64 );
				kdata = (kstat_named_t *) kstat_data_lookup( ksp, "cpu_nsec_user" );
				if (kdata != NULL)
					curr_cpu_nsecs_user = LOAD( kdata->value.ui64 );
				kdata = (kstat_named_t *) kstat_data_lookup( ksp, "cpu_nsec_wait" );
				if (kdata != NULL)
					curr_cpu_nsecs_wait = LOAD( kdata->value.ui64 );

				if (found == 2) {
					cpu_load[i].cpu_nsecs_idle = curr_cpu_nsecs_idle;
					cpu_load[i].cpu_nsecs_kernel = curr_cpu_nsecs_kernel;
					cpu_load[i].cpu_nsecs_user = curr_cpu_nsecs_user;
					cpu_load[i].cpu_nsecs_wait = curr_cpu_nsecs_wait;
					continue;
				}

				totalNsecs = (curr_cpu_nsecs_idle - cpu_load[i].cpu_nsecs_idle) +
				    (curr_cpu_nsecs_kernel - cpu_load[i].cpu_nsecs_kernel) +
				    (curr_cpu_nsecs_user - cpu_load[i].cpu_nsecs_user) +
				    (curr_cpu_nsecs_wait - cpu_load[i].cpu_nsecs_wait);

				if (totalNsecs > 10) {
					cpu_load[i].idle_load =
					    (100.0 * (curr_cpu_nsecs_idle - cpu_load[i].cpu_nsecs_idle)) \
					    / totalNsecs;
					cpu_load[i].kernel_load =
					    (100.0 * (curr_cpu_nsecs_kernel - cpu_load[i].cpu_nsecs_kernel)) \
					    / totalNsecs;
					cpu_load[i].user_load =
					    (100.0 * (curr_cpu_nsecs_user - cpu_load[i].cpu_nsecs_user)) \
					    / totalNsecs;
					cpu_load[i].wait_load =
					    (100.0 * (curr_cpu_nsecs_wait - cpu_load[i].cpu_nsecs_wait)) \
					    / totalNsecs;

					cpu_load[i].cpu_nsecs_idle = curr_cpu_nsecs_idle;
					cpu_load[i].cpu_nsecs_kernel = curr_cpu_nsecs_kernel;
					cpu_load[i].cpu_nsecs_user = curr_cpu_nsecs_user;
					cpu_load[i].cpu_nsecs_wait = curr_cpu_nsecs_wait;
				}
			}
		}
	}
	kstat_close( kctl );
#endif /* ! HAVE_KSTAT */

	return( 0 );
}

void printLoadAvg1Info( const char *cmd ) {
	fprintf(CurrentClient, "avnrun 1min\t0\t0\n" );
}

void printLoadAvg1( const char *cmd ) {
	fprintf(CurrentClient, "%f\n", loadavg1 );
}

void printLoadAvg5Info( const char *cmd ) {
	fprintf(CurrentClient, "avnrun 5min\t0\t0\n" );
}

void printLoadAvg5( const char *cmd ) {
	fprintf(CurrentClient, "%f\n", loadavg5 );
}

void printLoadAvg15Info( const char *cmd ) {
	fprintf(CurrentClient, "avnrun 15min\t0\t0\n" );
}

void printLoadAvg15( const char *cmd ) {
	fprintf(CurrentClient, "%f\n", loadavg15 );
}

void printCPUxUser( const char* cmd ) {
	int id;

	sscanf( cmd + 7, "%d", &id );
	fprintf(CurrentClient, "%f\n", cpu_load[ id ].user_load );
}

void printCPUxUserInfo( const char* cmd ) {
	int id;

	sscanf( cmd + 7, "%d", &id );
	fprintf(CurrentClient, "CPU %d User Load\t0\t100\t%%\n", id );
}

void printCPUxKernel( const char* cmd ) {
	int id;

	sscanf( cmd + 7, "%d", &id );
	fprintf(CurrentClient, "%f\n", cpu_load[ id ].kernel_load );
}

void printCPUxKernelInfo( const char* cmd ) {
	int id;

	sscanf( cmd + 7, "%d", &id );
	fprintf(CurrentClient, "CPU %d System Load\t0\t100\t%%\n", id );
}

void printCPUxTotalLoad( const char* cmd ) {
	int id;

	sscanf( cmd + 7, "%d", &id );
	fprintf(CurrentClient, "%f\n", cpu_load[ id ].user_load + \
	    cpu_load[ id ].kernel_load + cpu_load[ id ].wait_load );
}

void printCPUxTotalLoadInfo( const char* cmd ) {
	int id;

	sscanf( cmd + 7, "%d", &id );
	fprintf(CurrentClient, "CPU %d Total Load\t0\t100\t%%\n", id );
}

void printCPUxIdle( const char* cmd ) {
	int id;

	sscanf( cmd + 7, "%d", &id );
	fprintf(CurrentClient, "%f\n", cpu_load[ id ].idle_load );
}

void printCPUxIdleInfo( const char* cmd ) {
	int id;

	sscanf( cmd + 7, "%d", &id );
	fprintf(CurrentClient, "CPU %d Idle Load\t0\t100\t%%\n", id );
}

void printCPUxWait( const char* cmd )
{
	int id;

	sscanf( cmd + 7, "%d", &id );
	fprintf(CurrentClient, "%f\n", cpu_load[ id ].wait_load );
}

void printCPUxWaitInfo( const char* cmd )
{
	int id;

	sscanf( cmd + 7, "%d", &id );
	fprintf(CurrentClient, "CPU %d Wait Load\t0\t100\t%%\n", id );
}
