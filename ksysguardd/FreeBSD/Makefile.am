AM_CFLAGS = -Wall

INCLUDES = -DKSYSGUARDDRCFILE="\"$(KSYSGUARDDRCFILE)\"" -I$(srcdir)/../../CContLib -I$(srcdir)/..

noinst_LIBRARIES = libksysguardd.a
libksysguardd_a_SOURCES = CPU.c Memory.c ProcessList.c apm.c diskstat.c \
	loadavg.c logfile.c netdev.c
