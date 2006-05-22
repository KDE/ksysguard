INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

ADD_LIBRARY(libksysguardd STATIC 
            acpi.c
            apm.c
            cpuinfo.c
            diskstat.c
            i8k.c
            lmsensors.c
            loadavg.c
            logfile.c
            Memory.c
            netdev.c
            netstat.c
            ProcessList.c
            stat.c
            xres.c)

FIND_LIBRARY(XRES_LIBRARY libXRes)
if(${XRES_LIBRARY})
ADD_DEFINITIONS(-DHAVE_XRES)
endif(${XRES_LIBRARY})

if(${LMSENSORS_LIBRARY})
ADD_DEFINITIONS(-DHAVE_SENSORS_SENSORS_H)
endif(${LMSENSORS_LIBRARY})
TARGET_LINK_LIBRARIES(libksysguardd ${XRES_LIBRARY} ${LMSENSORS_LIBRARY})

IF(EXISTS /proc/i8k)
ADD_DEFINITIONS(-DHAVE_I8K_SUPPORT)
ENDIF(EXISTS /proc/i8k)

########### install files ###############




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
