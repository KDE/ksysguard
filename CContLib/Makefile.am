AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = conttest

lib_LIBRARIES = libccont.a
include_HEADERS = ccont.h

libccont_a_SOURCES = ccont.c dll.c pga.c sll.c

conttest_SOURCES = conttest.c
conttest_LDADD = -L. -lccont

conttest: libccont.a

CLEANFILES = *~ 
