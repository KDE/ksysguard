#
#  $Id$
#
CFLAGS = -Wall $(CONFIGFLAGS)
KSYSGUARDDRCFILE=@sysconfdir@/ksysguarddrc

INCLUDES = -DKSYSGUARDDRCFILE="\"$(KSYSGUARDDRCFILE)\"" -I$(srcdir)/../../CContLib

bin_PROGRAMS = ksysguardd

ksysguardd_SOURCES = ksysguardd.c Command.c Memory.c \
	LoadAvg.c ProcessList.c NetDev.c

ksysguardd_LDADD = -L../../CContLib -lccont -lkstat -lsocket
