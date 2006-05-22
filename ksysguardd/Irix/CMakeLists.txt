INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

ADD_LIBRARY(libksysguardd STATIC 
            cpu.c
            LoadAvg.c
            Memory.c
            NetDev.c
            ProcessList.c)


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../../CContLib -I..
#
#noinst_LIBRARIES = libksysguardd.a
#libksysguardd_a_SOURCES = Memory.c LoadAvg.c ProcessList.c NetDev.c cpu.c
