CPPFLAGS = -D_GNU_SOURCE
CFLAGS = -Wall $(CONFIGFLAGS)
KSYSGUARDDRCFILE=@sysconfdir@/ksysguarddrc

INCLUDES = -DKSYSGUARDDRCFILE="\"$(KSYSGUARDDRCFILE)\"" -I$(srcdir)/../../CContLib

bin_PROGRAMS = ksysguardd

ksysguardd_SOURCES = ksysguardd.c Command.c ProcessList.c \
	Memory.c stat.c netdev.c PWUIDCache.c apm.c loadavg.c cpuinfo.c \
	lmsensors.c netstat.c diskstat.c logfile.c conf.c

ksysguardd_LDADD = -L../../CContLib -lccont $(LIBSENSORS)
