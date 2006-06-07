INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

ADD_LIBRARY(libksysguardd STATIC 
            cpu.c
            memory.c
            ProcessList.c)


########### install files ###############





#original Makefile.am contents follow:

#AM_CFLAGS = -Wall
#
#INCLUDES = -I$(srcdir)/../../CContLib -I$(srcdir)/..
#
#noinst_LIBRARIES = libksysguardd.a
#libksysguardd_a_SOURCES = cpu.c memory.c ProcessList.c
