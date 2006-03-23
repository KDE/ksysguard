kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##
##
#AM_CFLAGS = -Wall 
#
#INCLUDES = -I$(srcdir)/../../CContLib -I..
#
#lib_LIBRARIES = libksysguardd.a
#libksysguardd_a_SOURCES = Memory.c LoadAvg.c NetDev.c
