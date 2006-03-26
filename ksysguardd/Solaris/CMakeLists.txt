kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##
##
#
## Sun's C++ compiler doesn't support -Wall
##AM_CFLAGS = -Wall 
#
#INCLUDES = -I$(srcdir)/../../CContLib -I$(srcdir)/..
#
#noinst_LIBRARIES = libksysguardd.a
#libksysguardd_a_SOURCES = Memory.c LoadAvg.c ProcessList.c NetDev.c
