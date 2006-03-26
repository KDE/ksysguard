kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../../CContLib -I..
#
#noinst_LIBRARIES = libksysguardd.a
#libksysguardd_a_SOURCES = Memory.c LoadAvg.c ProcessList.c NetDev.c cpu.c
