INCLUDES = -I../../CContLib
CFLAGS = -Wall $(CONFIGFLAGS)

bin_PROGRAMS = ktopd

ktopd_SOURCES = ktopd.c Command.c Dispatcher.c ProcessList.c Memory.c CPU.c
ktopd_LDADD = -L../../CContLib -lccont
