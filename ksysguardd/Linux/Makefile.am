INCLUDES = -I$(srcdir)/../../CContLib
CPPFLAGS = -D_GNU_SOURCE
CFLAGS = -Wall $(CONFIGFLAGS)

bin_PROGRAMS = ksysguardd

ksysguardd_SOURCES = ksysguardd.c Command.c ProcessList.c \
	Memory.c stat.c netdev.c PWUIDCache.c apm.c loadavg.c cpuinfo.c \
	lmsensors.c netstat.c diskstat.c

ksysguardd_LDADD = -L../../CContLib -lccont $(LIBSENSORS)
