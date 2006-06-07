INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

ADD_LIBRARY(libksysguardd STATIC 
            LoadAvg.c
            Memory.c
            NetDev.c)


########### install files ###############





#original Makefile.am contents follow:

##
##
#AM_CFLAGS = -Wall 
#
#INCLUDES = -I$(srcdir)/../../CContLib -I..
#
#lib_LIBRARIES = libksysguardd.a
#libksysguardd_a_SOURCES = Memory.c LoadAvg.c NetDev.c
