/*
    KTop, the KDE Task Manager
   
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stropts.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>

#include "config.h"

#ifdef HAVE_KSTAT
#include <kstat.h>
#endif

#include "Command.h"
#include "NetDev.h"

/*
 *  available network interface statistics through kstat(3):
 *
 *	kstat name		value
 *  ----------------------------------------------------------------------
 *  for the loopback interface(s) we can get
 *	ipackets		# packets received
 *	opackets		# packets sent
 *  in addition to those, for "real" interfaces:
 *	oerrors			# xmit errors
 *	ierrors			# recv errors
 *	macxmt_errors		# xmit errors reported by hardware?
 *	macrcv_errors		# recv errors reported by hardware?
 *	opackets64		same as opackets (64bit)
 *	ipackets64		same as ipackets (64bit)
 *	obytes			# bytes sent
 *	rbytes			# bytes received
 *	obytes64		same as obytes (64bit)
 *	rbytes64		same as ibytes (64bit)
 *	collisions		# collisions
 *	multixmt		# multicasts sent?
 *	multircv		# multicasts received?
 *	brdcstxmt		# broadcasts transmitted
 *	brdcstrcv		# broadcasts received
 *	unknowns
 *	blocked
 *	ex_collisions
 *	defer_xmts
 *	align_errors
 *	fcs_errors
 *	oflo			# overflow errors
 *	uflo			# underflow errors
 *	runt_errors
 *	missed
 *	tx_late_collisions
 *	carrier_errors
 *	noxmtbuf
 *	norcvbuf
 *	xmt_badinterp
 *	rcv_badinterp
 *	intr			# interrupts?
 *	xmtretry		# xmit retries?
 *	ifspeed			interface speed: 10000000 for 10BaseT
 *	duplex			"half" or "full"
 *	media			e.g. "PHY/MII"
 *	promisc			promiscous mode (e.g. "off")
 *	first_collisions
 *	multi_collisions
 *	sqe_errors
 *	toolong_errors
 */

typedef struct {
	char		*Name;
	short		flags;
	unsigned long	ipackets;
	unsigned long	OLDipackets;
	unsigned long	opackets;
	unsigned long	OLDopackets;
	unsigned long	ierrors;
	unsigned long	OLDierrors;
	unsigned long	oerrors;
	unsigned long	OLDoerrors;
	unsigned long	collisions;
	unsigned long	OLDcollisions;
	unsigned long	multixmt;
	unsigned long	OLDmultixmt;
	unsigned long	multircv;
	unsigned long	OLDmultircv;
	unsigned long	brdcstxmt;
	unsigned long	OLDbrdcstxmt;
	unsigned long	brdcstrcv;
	unsigned long	OLDbrdcstrcv;
} NetDevInfo;


#define printerr(a) write(STDERR_FILENO, (a), strlen(a))

#define NBUFFERS 64
#define MAXNETDEVS 64
static NetDevInfo IfInfo[MAXNETDEVS];

static int NetDevCount;

/*
 *  insertnetdev()  --  insert device name & flags into our list
 */
int insertnetdev( const char *name, const short flags ) {

	int	i = 0;

	/*
	 *  interface "aliases" don't seem to have
	 *  separate kstat statistics, so we skip them
	 */
	if( strchr( name, (int) ':' ) != NULL )
		return( 0 );

	while( (i < NetDevCount) && (strcmp( IfInfo[i].Name, name ) != 0) ) {
		if( strcmp( IfInfo[i].Name, name ) == 0 )
			return( 0 );
		i++;
	}

	/*
	 *  init new slot
	 */
	IfInfo[i].Name = strdup( name );
	IfInfo[i].flags = flags;
	IfInfo[i].ipackets = 0L;
	IfInfo[i].OLDipackets = 0L;
	IfInfo[i].opackets = 0L;
	IfInfo[i].OLDopackets = 0L;
	IfInfo[i].ierrors = 0L;
	IfInfo[i].OLDierrors = 0L;
	IfInfo[i].oerrors = 0L;
	IfInfo[i].OLDoerrors = 0L;
	IfInfo[i].collisions = 0L;
	IfInfo[i].OLDcollisions = 0L;
	IfInfo[i].multixmt = 0L;
	IfInfo[i].OLDmultixmt = 0L;
	IfInfo[i].multircv = 0L;
	IfInfo[i].OLDmultircv = 0L;
	IfInfo[i].brdcstxmt = 0L;
	IfInfo[i].OLDbrdcstxmt = 0L;
	IfInfo[i].brdcstrcv = 0L;
	IfInfo[i].OLDbrdcstrcv = 0L;
	NetDevCount = ++i;

	/*  XXX: need sanity checks!  */
	return( 0 );
}

/*
 *  getnetdevlist()  --  get a list of all "up" interfaces
 */
int getnetdevlist( void ) {

	int		fd;
	int		buffsize;
	int		prevsize;
	int		prevCount;
	struct ifconf	ifc;
	struct ifreq	*ifr;

	if( (fd = socket( PF_INET, SOCK_DGRAM, 0 )) < 0 ) {
		return( -1 );
	}

	/*
	 *  get the interface list via iotl( SIOCGIFCONF )
	 *  the following algorithm based on ideas from W.R. Stevens'
	 *  "UNIX Network Programming", Vol. 1:
	 *  Since the ioctl may return 0, indicating success, even if the
	 *  ifreq buffer was too small, we have to make sure, it didn't
	 *  get truncated by comparing our initial size guess with the
	 *  actual returned size.
	 */
	prevsize = 0;
	buffsize = NBUFFERS * sizeof( struct ifreq );
	while( 1 ) {
		if( (ifc.ifc_buf = malloc( buffsize )) == NULL )
			return( -1 );

		ifc.ifc_len = buffsize;
		if( ioctl( fd, SIOCGIFCONF, &ifc ) < 0 ) {
			if( errno != EINVAL || prevsize != 0 ) {
				free( ifc.ifc_buf );
				return( -1 );
			}
		} else {
			if( ifc.ifc_len == prevsize )
				/*  success  */
				break;
			prevsize = ifc.ifc_len;
		}
		/*
		 *  initial buffer guessed too small, allocate a bigger one
		 */
		free( ifc.ifc_buf );
		buffsize = (NBUFFERS + 10) * sizeof( struct ifreq );
	}

	/*
	 *  get the names for all interfaces which are configured "up"
	 *  we're not interested in the ifc data (address), so we reuse the
	 *  same structure (with ifc.len set) for the next ioctl()
	 */
	prevCount = NetDevCount;
	for( ifr = (struct ifreq *) ifc.ifc_buf;
			ifr < (struct ifreq *) (ifc.ifc_buf + ifc.ifc_len);
			ifr++ ) {
		if( ioctl( fd, SIOCGIFFLAGS, ifr ) < 0 ) {
			free( ifc.ifc_buf );
			return( -1 );
		}
		if( ifr->ifr_flags & IFF_UP )
			insertnetdev( ifr->ifr_name, ifr->ifr_flags );
	}
	free( ifc.ifc_buf );
	close( fd );

	if( (prevCount > 0) && (prevCount != NetDevCount) ) {
		printerr( "RECONFIGURE\n" );
		prevCount = NetDevCount;
	}

	return( NetDevCount );
}

void initNetDev( void ) {
#ifdef HAVE_KSTAT
	char	mon[128];
	int	i;

	getnetdevlist();
	for( i = 0; i < NetDevCount; i++ ) {
		sprintf( mon, "network/%s/ipackets", IfInfo[i].Name );
		registerMonitor( mon, "integer",
					printIPackets, printIPacketsInfo );
		sprintf( mon, "network/%s/opackets", IfInfo[i].Name );
		registerMonitor( mon, "integer",
					printOPackets, printOPacketsInfo );
		/*
		 *  if this isn't a loopback interface,
		 *  register additional monitors
		 */
		if( ! (IfInfo[i].flags & IFF_LOOPBACK) ) {
			/*
			 *  recv errors
			 */
			sprintf( mon, "network/%s/ierrors",
					IfInfo[i].Name );
			registerMonitor( mon, "integer",
					printIErrors, printIErrorsInfo );
			/*
			 *  xmit errors
			 */
			sprintf( mon, "network/%s/oerrors",
					IfInfo[i].Name );
			registerMonitor( mon, "integer",
					printOErrors, printOErrorsInfo );
			/*
			 *  collisions
			 */
			sprintf( mon, "network/%s/collisions",
					IfInfo[i].Name );
			registerMonitor( mon, "integer",
					printCollisions, printCollisionsInfo );
			/*
			 *  multicast xmits
			 */
			sprintf( mon, "network/%s/multixmt",
					IfInfo[i].Name );
			registerMonitor( mon, "integer",
					printMultiXmits, printMultiXmitsInfo );
			/*
			 *  multicast recvs
			 */
			sprintf( mon, "network/%s/multircv",
					IfInfo[i].Name );
			registerMonitor( mon, "integer",
					printMultiRecvs, printMultiRecvsInfo );
			/*
			 *  broadcast xmits
			 */
			sprintf( mon, "network/%s/brdcstxmt",
					IfInfo[i].Name );
			registerMonitor( mon, "integer",
					printBcastXmits, printBcastXmitsInfo );
			/*
			 *  broadcast recvs
			 */
			sprintf( mon, "network/%s/brdcstrcv",
					IfInfo[i].Name );
			registerMonitor( mon, "integer",
					printBcastRecvs, printBcastRecvsInfo );
		}
	}
#endif
}

void exitNetDev( void ) {
}

int updateNetDev( void ) {

#ifdef HAVE_KSTAT
	kstat_ctl_t		*kctl;
	kstat_t			*ksp;
	kstat_named_t		*kdata;
	int			i;

	/*
	 *  get a kstat handle and update the user's kstat chain
	 */
	if( (kctl = kstat_open()) == NULL )
		return( 0 );
	while( kstat_chain_update( kctl ) != 0 )
		;

	for( i = 0; i < NetDevCount; i++ ) {
		char	*name;
		char	*ptr;

		/*
		 *  chop off the trailing interface no
		 */
		name = strdup( IfInfo[i].Name );
		ptr = name + strlen( name ) - 1;
		while( (ptr > name) && isdigit( (int) *ptr ) ) {
			*ptr = '\0';
			ptr--;
		}

		/*
		 *  traverse the kstat chain
		 *  to find the appropriate statistics
		 */
		if( (ksp = kstat_lookup( kctl,
				name, 0, IfInfo[i].Name )) == NULL ) {
			free( name );
			return( 0 );
		}
		if( kstat_read( kctl, ksp, NULL ) == -1 ) {
			free( name );
			return( 0 );
		}
		free( name );

		/*
		 *  lookup & store the data
		 */
		kdata = (kstat_named_t *) kstat_data_lookup( ksp, "ipackets" );
		if( kdata != NULL ) {
			IfInfo[i].OLDipackets = IfInfo[i].ipackets;
			IfInfo[i].ipackets = kdata->value.ul;
		}
		kdata = (kstat_named_t *) kstat_data_lookup( ksp, "opackets" );
		if( kdata != NULL ) {
			IfInfo[i].OLDopackets = IfInfo[i].opackets;
			IfInfo[i].opackets = kdata->value.ul;
		}
		kdata = (kstat_named_t *) kstat_data_lookup( ksp, "ierrors" );
		if( kdata != NULL ) {
			IfInfo[i].OLDierrors = IfInfo[i].ierrors;
			IfInfo[i].ierrors = kdata->value.ul;
		}
		kdata = (kstat_named_t *) kstat_data_lookup( ksp, "oerrors" );
		if( kdata != NULL ) {
			IfInfo[i].OLDoerrors = IfInfo[i].oerrors;
			IfInfo[i].oerrors = kdata->value.ul;
		}
		kdata = (kstat_named_t *) kstat_data_lookup( ksp, "collisions" );
		if( kdata != NULL ) {
			IfInfo[i].OLDcollisions = IfInfo[i].collisions;
			IfInfo[i].collisions = kdata->value.ul;
		}
		kdata = (kstat_named_t *) kstat_data_lookup( ksp, "multixmt" );
		if( kdata != NULL ) {
			IfInfo[i].OLDmultixmt = IfInfo[i].multixmt;
			IfInfo[i].multixmt = kdata->value.ul;
		}
		kdata = (kstat_named_t *) kstat_data_lookup( ksp, "multircv" );
		if( kdata != NULL ) {
			IfInfo[i].OLDmultircv = IfInfo[i].multircv;
			IfInfo[i].multircv = kdata->value.ul;
		}
		kdata = (kstat_named_t *) kstat_data_lookup( ksp, "brdcstxmt" );
		if( kdata != NULL ) {
			IfInfo[i].OLDbrdcstxmt = IfInfo[i].brdcstxmt;
			IfInfo[i].brdcstxmt = kdata->value.ul;
		}
		kdata = (kstat_named_t *) kstat_data_lookup( ksp, "brdcstrcv" );
		if( kdata != NULL ) {
			IfInfo[i].OLDbrdcstrcv = IfInfo[i].brdcstrcv;
			IfInfo[i].brdcstrcv = kdata->value.ul;
		}
	}

	kstat_close( kctl );
#endif /* ! HAVE_KSTAT */

	return( 0 );
}

void printIPacketsInfo( const char *cmd ) {
	printf( "Received Packets\t0\t0\tPackets\n" );
}

void printIPackets( const char *cmd ) {

	char	*cmdcopy = strdup( cmd );
	char	*name, *ptr;
	int	i;

	ptr = strchr( cmdcopy, (int) '/' );
	name = ++ptr;
	ptr = strchr( name, (int) '/' );
	*ptr = '\0';

	for( i = 0; i < NetDevCount; i++ ) {
		if( (IfInfo[i].OLDipackets > 0)
				&& (strcmp( IfInfo[i].Name, name ) == 0) ) {
			printf( "%ld\n",
				IfInfo[i].ipackets - IfInfo[i].OLDipackets);
			free( cmdcopy );
			return;
		}
	}
	free( cmdcopy );
	printf( "0\n" );
}

void printOPacketsInfo( const char *cmd ) {
	printf( "Transmitted Packets\t0\t0\tPackets\n" );
}

void printOPackets( const char *cmd ) {

	char	*cmdcopy = strdup( cmd );
	char	*name, *ptr;
	int	i;

	ptr = strchr( cmdcopy, (int) '/' );
	name = ++ptr;
	ptr = strchr( name, (int) '/' );
	*ptr = '\0';

	for( i = 0; i < NetDevCount; i++ ) {
		if( (IfInfo[i].OLDopackets > 0)
				&& (strcmp( IfInfo[i].Name, name ) == 0) ) {
			printf( "%ld\n",
				IfInfo[i].opackets - IfInfo[i].OLDopackets );
			free( cmdcopy );
			return;
		}
	}
	free( cmdcopy );
	printf( "0\n" );
}

void printIErrorsInfo( const char *cmd ) {
	printf( "Input Errors\t0\t0\tPackets\n" );
}

void printIErrors( const char *cmd ) {

	char	*cmdcopy = strdup( cmd );
	char	*name, *ptr;
	int	i;

	ptr = strchr( cmdcopy, (int) '/' );
	name = ++ptr;
	ptr = strchr( name, (int) '/' );
	*ptr = '\0';

	for( i = 0; i < NetDevCount; i++ ) {
		if( (IfInfo[i].OLDierrors > 0)
				&& (strcmp( IfInfo[i].Name, name ) == 0) ) {
			printf( "%ld\n",
				IfInfo[i].ierrors - IfInfo[i].OLDierrors );
			free( cmdcopy );
			return;
		}
	}
	free( cmdcopy );
	printf( "0\n" );
}

void printOErrorsInfo( const char *cmd ) {
	printf( "Output Errors\t0\t0\tPackets\n" );
}

void printOErrors( const char *cmd ) {

	char	*cmdcopy = strdup( cmd );
	char	*name, *ptr;
	int	i;

	ptr = strchr( cmdcopy, (int) '/' );
	name = ++ptr;
	ptr = strchr( name, (int) '/' );
	*ptr = '\0';

	for( i = 0; i < NetDevCount; i++ ) {
		if( (IfInfo[i].OLDoerrors > 0)
				&& (strcmp( IfInfo[i].Name, name ) == 0) ) {
			printf( "%ld\n",
				IfInfo[i].oerrors - IfInfo[i].OLDoerrors );
			free( cmdcopy );
			return;
		}
	}
	free( cmdcopy );
	printf( "0\n" );
}

void printCollisionsInfo( const char *cmd ) {
	printf( "Collisions\t0\t0\tPackets\n" );
}

void printCollisions( const char *cmd ) {

	char	*cmdcopy = strdup( cmd );
	char	*name, *ptr;
	int	i;

	ptr = strchr( cmdcopy, (int) '/' );
	name = ++ptr;
	ptr = strchr( name, (int) '/' );
	*ptr = '\0';

	for( i = 0; i < NetDevCount; i++ ) {
		if( (IfInfo[i].OLDcollisions > 0)
				&& (strcmp( IfInfo[i].Name, name ) == 0) ) {
			printf( "%ld\n",
				IfInfo[i].collisions - IfInfo[i].OLDcollisions );
			free( cmdcopy );
			return;
		}
	}
	free( cmdcopy );
	printf( "0\n" );
}

void printMultiXmitsInfo( const char *cmd ) {
	printf( "Multicasts Sent\t0\t0\tPackets\n" );
}

void printMultiXmits( const char *cmd ) {

	char	*cmdcopy = strdup( cmd );
	char	*name, *ptr;
	int	i;

	ptr = strchr( cmdcopy, (int) '/' );
	name = ++ptr;
	ptr = strchr( name, (int) '/' );
	*ptr = '\0';

	for( i = 0; i < NetDevCount; i++ ) {
		if( (IfInfo[i].OLDmultixmt > 0)
				&& (strcmp( IfInfo[i].Name, name ) == 0) ) {
			printf( "%ld\n",
				IfInfo[i].multixmt - IfInfo[i].OLDmultixmt );
			free( cmdcopy );
			return;
		}
	}
	free( cmdcopy );
	printf( "0\n" );
}

void printMultiRecvsInfo( const char *cmd ) {
	printf( "Multicasts Received\t0\t0\tPackets\n" );
}

void printMultiRecvs( const char *cmd ) {

	char	*cmdcopy = strdup( cmd );
	char	*name, *ptr;
	int	i;

	ptr = strchr( cmdcopy, (int) '/' );
	name = ++ptr;
	ptr = strchr( name, (int) '/' );
	*ptr = '\0';

	for( i = 0; i < NetDevCount; i++ ) {
		if( (IfInfo[i].OLDmultircv > 0)
				&& (strcmp( IfInfo[i].Name, name ) == 0) ) {
			printf( "%ld\n",
				IfInfo[i].multircv - IfInfo[i].OLDmultircv );
			free( cmdcopy );
			return;
		}
	}
	free( cmdcopy );
	printf( "0\n" );
}

void printBcastXmitsInfo( const char *cmd ) {
	printf( "Broadcasts Sent\t0\t0\tPackets\n" );
}

void printBcastXmits( const char *cmd ) {

	char	*cmdcopy = strdup( cmd );
	char	*name, *ptr;
	int	i;

	ptr = strchr( cmdcopy, (int) '/' );
	name = ++ptr;
	ptr = strchr( name, (int) '/' );
	*ptr = '\0';

	for( i = 0; i < NetDevCount; i++ ) {
		if( (IfInfo[i].OLDbrdcstxmt > 0)
				&& (strcmp( IfInfo[i].Name, name ) == 0) ) {
			printf( "%ld\n",
				IfInfo[i].brdcstxmt - IfInfo[i].OLDbrdcstxmt );
			free( cmdcopy );
			return;
		}
	}
	free( cmdcopy );
	printf( "0\n" );
}

void printBcastRecvsInfo( const char *cmd ) {
	printf( "Broadcasts Received\t0\t0\tPackets\n" );
}

void printBcastRecvs( const char *cmd ) {

	char	*cmdcopy = strdup( cmd );
	char	*name, *ptr;
	int	i;

	ptr = strchr( cmdcopy, (int) '/' );
	name = ++ptr;
	ptr = strchr( name, (int) '/' );
	*ptr = '\0';

	for( i = 0; i < NetDevCount; i++ ) {
		if( (IfInfo[i].OLDbrdcstrcv > 0)
				&& (strcmp( IfInfo[i].Name, name ) == 0) ) {
			printf( "%ld\n",
				IfInfo[i].brdcstrcv - IfInfo[i].OLDbrdcstrcv );
			free( cmdcopy );
			return;
		}
	}
	free( cmdcopy );
	printf( "0\n" );
}
