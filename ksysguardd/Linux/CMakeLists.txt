kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -D_GNU_SOURCE
#AM_CFLAGS = -Wall 
#
## add all supported modules
#if supports_i8k
#KSGRD_SUPPORTS = -DHAVE_I8K_SUPPORT
#endif
#
#
#INCLUDES = -I$(srcdir)/../../CContLib -I$(srcdir)/.. $(KSGRD_SUPPORTS) $(all_includes)
#
#noinst_LIBRARIES = libksysguardd.a
#libksysguardd_a_SOURCES = ProcessList.c	Memory.c stat.c netdev.c apm.c acpi.c \
#	loadavg.c cpuinfo.c lmsensors.c netstat.c diskstat.c logfile.c i8k.c
