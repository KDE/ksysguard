include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib/ )

ADD_DEFINITIONS( -DKSYSGUARDDRCFILE=\"\\"${CMAKE_INSTALL_PREFIX}${SYSCONF_INSTALL_DIR}/ksysguarddrc\\"\" )

ADD_DEFINITIONS(-DOSTYPE_${CMAKE_SYSTEM_NAME})
add_subdirectory( ${CMAKE_SYSTEM_NAME} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME} )


########### next target ###############

set(libccont_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/../CContLib/ccont.c )

set(ksysguardd_SRCS ${libccont_SRCS}
   Command.c 
   conf.c 
   ksysguardd.c 
   PWUIDCache.c )

kde4_automoc(${ksysguardd_SRCS})

kde4_add_executable(ksysguardd ${ksysguardd_SRCS})

target_link_libraries(ksysguardd  ${KDE4_KDECORE_LIBS} kdefakes_nonpic )

ADD_DEPENDENCIES(ksysguardd libksysguardd)
TARGET_LINK_LIBRARIES(ksysguardd libksysguardd)

install(TARGETS ksysguardd DESTINATION bin)


########### install files ###############





#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = foreign
#
#if include_ksysguardd_linux
#linux_SUBDIR = Linux
#endif
#if include_ksysguardd_freebsd
#freebsd_SUBDIR = FreeBSD
#endif
#if include_ksysguardd_netbsd
#netbsd_SUBDIR = NetBSD
#endif
#if include_ksysguardd_solaris
#solaris_SUBDIR = Solaris
#endif
#if include_ksysguardd_tru64
#tru64_SUBDIR = Tru64
#endif
#if include_ksysguardd_irix
#irix_SUBDIR = Irix
#endif
#if include_ksysguardd_openbsd
#openbsd_SUBDIR = OpenBSD
#endif
#
#SUBDIRS = $(linux_SUBDIR) $(freebsd_SUBDIR) $(netbsd_SUBDIR)\
#	$(solaris_SUBDIR) $(tru64_SUBDIR) $(irix_SUBDIR) $(openbsd_SUBDIR)
#
#KSYSGUARDDRCFILE=$(sysconfdir)/ksysguarddrc
#INCLUDES = -DKSYSGUARDDRCFILE="\"$(KSYSGUARDDRCFILE)\"" -DOSTYPE_$(UNAME) -I$(srcdir)/../CContLib -I$(srcdir)/$(UNAME)
#
#bin_PROGRAMS = ksysguardd
#
#ksysguardd_SOURCES = Command.c conf.c ksysguardd.c PWUIDCache.c
#ksysguardd_LDFLAGS = $(all_libraries)
#ksysguardd_LDADD =  $(top_builddir)/workspace/ksysguard/ksysguardd/$(UNAME)/libksysguardd.a ../CContLib/libccont.a -lkdefakes_nonpic $(LIBHOSTS)
