INCLUDES = -I$(srcdir)/../../CContLib
CFLAGS = -Wall $(CONFIGFLAGS)

bin_PROGRAMS = ksysguardd

ksysguardd_SOURCES = ksysguardd.c Command.c Dispatcher.c ProcessList.c \
	Memory.c CPU.c
ksysguardd_LDADD = -L../../CContLib -lccont -lkvm
