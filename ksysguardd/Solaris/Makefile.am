#
#  $Id$
#
INCLUDES = -I$(srcdir)/../../CContLib
CFLAGS = -Wall $(CONFIGFLAGS)

bin_PROGRAMS = ksysguardd

ksysguardd_SOURCES = ksysguardd.c Command.c Dispatcher.c Memory.c \
	LoadAvg.c ProcessList.c NetDev.c

ksysguardd_LDADD = -L../../CContLib -lccont -lkstat -lsocket
