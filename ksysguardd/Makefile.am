AUTOMAKE_OPTIONS = foreign

SUBDIRS = $(UNAME)

KSYSGUARDDRCFILE=@sysconfdir@/ksysguarddrc
INCLUDES = -DKSYSGUARDDRCFILE="\"$(KSYSGUARDDRCFILE)\"" -DOSTYPE_$(UNAME) -I$(srcdir)/../CContLib -I$(srcdir)/$(UNAME)

bin_PROGRAMS = ksysguardd

ksysguardd_SOURCES = Command.c conf.c ksysguardd.c PWUIDCache.c
ksysguardd_LDFLAGS = -L../CContLib
ksysguardd_LDADD =  $(UNAME)/libksysguardd.a -lccont $(LIBHOSTS)


