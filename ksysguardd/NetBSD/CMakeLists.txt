INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

ADD_LIBRARY(libksysguardd STATIC 
            apm.c
            CPU.c
            diskstat.c
            loadavg.c
            logfile.c
            Memory.c
            netdev.c
            ProcessList.c)

########### install files ###############





#original Makefile.am contents follow:

##
##
#
#INCLUDES = -I$(srcdir)/../../CContLib -I$(srcdir)/..
#
#noinst_LIBRARIES = libksysguardd.a
#libksysguardd_a_SOURCES = CPU.c Memory.c ProcessList.c apm.c diskstat.c \
#	loadavg.c logfile.c netdev.c
