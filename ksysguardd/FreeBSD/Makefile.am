INCLUDES = -I$(srcdir)/../../CContLib
CFLAGS = -Wall $(CONFIGFLAGS)
KSYSGUARDDRCFILE=@sysconfdir@/ksysguarddrc

INCLUDES = -DKSYSGUARDDRCFILE="\"$(KSYSGUARDDRCFILE)\"" -I$(srcdir)/../../CContLib

bin_PROGRAMS = ksysguardd

ksysguardd_SOURCES = CPU.c Command.c Memory.c ProcessList.c apm.c diskstat.c \
	ksysguardd.c loadavg.c logfile.c netdev.c conf.c
ksysguardd_LDADD = -L../../CContLib -lccont -lkvm
