INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../CContLib ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/.. )

SET(LIBKSYSGUARDD_FILES
            acpi.c
            apm.c
            cpuinfo.c
            diskstat.c
            i8k.c
            loadavg.c
            logfile.c
            Memory.c
            netdev.c
            netstat.c
            ProcessList.c
            stat.c)

if(XRES_LIBRARY)
  SET(LIBKSYSGUARDD_FILES ${LIBKSYSGUARDD_FILES} xres.c)
endif(XRES_LIBRARY)

if(SENSORS_FOUND)
  SET(LIBKSYSGUARDD_FILES ${LIBKSYSGUARDD_FILES} lmsensors.c)
endif(SENSORS_FOUND)

if(EXISTS /proc/i8k)
  ADD_DEFINITIONS(-DHAVE_I8K_SUPPORT)
ENDIF(EXISTS /proc/i8k)


ADD_LIBRARY(libksysguardd STATIC 
            ${LIBKSYSGUARDD_FILES})
if(XRES_LIBRARY)
  TARGET_LINK_LIBRARIES(libksysguardd ${XRES_LIBRARY})
endif(XRES_LIBRARY)
if(SENSORS_FOUND)
  TARGET_LINK_LIBRARIES(libksysguardd ${SENSORS_LIBRARY})
endif(SENSORS_FOUND)


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
