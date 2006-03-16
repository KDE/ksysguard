kde4_header()

project(ksysguard)

add_subdirectory( CContLib ) 
add_subdirectory( gui ) 
add_subdirectory( pics ) 
add_subdirectory( example ) 
# Laurent 'readd test for it'
#add_subdirectory( ksysguardd )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#if include_ksysguardd
#KSYSGUARDD_DIR = ksysguardd
#endif
#
## claim, which subdirectories you want to install
#SUBDIRS = CContLib $(KSYSGUARDD_DIR) gui pics example
#
#KDE_ICON = ksysguard
#
