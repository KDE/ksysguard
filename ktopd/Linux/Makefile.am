INCLUDES = -I$(srcdir)/../../CContLib
CFLAGS = -Wall $(CONFIGFLAGS)

bin_PROGRAMS = ktopd

ktopd_SOURCES = ktopd.c Command.c Dispatcher.c ProcessList.c Memory.c stat.c
ktopd_LDADD = -L../../CContLib -lccont
