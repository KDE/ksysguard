INCLUDES = -I$(srcdir)/../../CContLib
CFLAGS = -Wall $(CONFIGFLAGS)

bin_PROGRAMS = ksysguardd

ksysguardd_SOURCES = CPU.c Command.c Memory.c ProcessList.c apm.c diskstat.c \
	ksysguardd.c loadavg.c logfile.c netdev.c 
ksysguardd_LDADD = -L../../CContLib -lccont -lkvm
