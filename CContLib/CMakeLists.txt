kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = foreign
#
#noinst_LIBRARIES = libccont.a
#noinst_HEADERS = ccont.h
#
#libccont_a_SOURCES = ccont.c
