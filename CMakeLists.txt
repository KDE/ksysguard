
project(ksysguard)
message(STATUS "Building KSysGuard")
add_subdirectory( CContLib )
add_subdirectory( gui )
add_subdirectory( pics )
add_subdirectory( example )
add_subdirectory( ksysguardd )



########### install files ###############


kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )




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
