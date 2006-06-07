AM_CPPFLAGS = -D_GNU_SOURCE
AM_CFLAGS = -Wall

# add all supported modules
if supports_i8k
KSGRD_SUPPORTS = -DHAVE_I8K_SUPPORT
endif

if supports_xres
XRES_DEFINE = -DHAVE_XRES
endif

INCLUDES = -I$(srcdir)/../../CContLib -I$(srcdir)/.. $(KSGRD_SUPPORTS) $(XRES_DEFINE) $(all_includes)

noinst_LIBRARIES = libksysguardd.a
libksysguardd_a_SOURCES = ProcessList.c	Memory.c stat.c netdev.c apm.c acpi.c \
	loadavg.c cpuinfo.c lmsensors.c netstat.c diskstat.c logfile.c i8k.c \
	xres.c
