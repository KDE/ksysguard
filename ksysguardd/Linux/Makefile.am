CPPFLAGS = -D_GNU_SOURCE
CFLAGS = -Wall $(CONFIGFLAGS)

INCLUDES = -I$(srcdir)/../../CContLib -I..

lib_LIBRARIES = libksysguardd.a
libksysguardd_a_SOURCES = ProcessList.c	Memory.c stat.c netdev.c apm.c \
	loadavg.c cpuinfo.c lmsensors.c netstat.c diskstat.c logfile.c
