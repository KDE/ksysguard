kde4_header()

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(linux_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(freebsd_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(netbsd_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(solaris_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(tru64_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(irix_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(openbsd_SUBDIR)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(ksysguardd_SRCS 
   Command.c 
   conf.c 
   ksysguardd.c 
   PWUIDCache.c )

kde4_automoc(${ksysguardd_SRCS})

kde4_add_executable(ksysguardd ${ksysguardd_SRCS})

target_link_libraries(ksysguardd  ${QT_AND_KDECORE_LIBS} kdefakes_nonpic )

install_targets(/bin ksysguardd )


########### install files ###############


kde4_footer()



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
