# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.3 from Makefile.am

# Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

# add here all files


SHELL = /bin/sh

srcdir = .
top_srcdir = ../..
prefix = /home/kulow/prod/KDE
exec_prefix = ${prefix}

bindir = /home/kulow/prod/KDE/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DISTDIR =

pkgdatadir = $(datadir)/kdenonbeta
pkglibdir = $(libdir)/kdenonbeta
pkgincludedir = $(includedir)/kdenonbeta

top_builddir = ../..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /home/kulow/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_alias = sparc-sun-solaris2.5.1
build_triplet = sparc-sun-solaris2.5.1
host_alias = sparc-sun-solaris2.5.1
host_triplet = sparc-sun-solaris2.5.1
target_alias = sparc-sun-solaris2.5.1
target_triplet = sparc-sun-solaris2.5.1
CC = gcc
CPP = gcc -E
CXX = g++
GLINC = @GLINC@
GLLIB = @GLLIB@
GMSGFMT = /home/kulow/bin/msgfmt
KDE_EXTRA_RPATH = 
KDE_INCLUDES = -I/home/kulow/prod/KDE/include
KDE_LDFLAGS = -L/home/kulow/prod/KDE/lib
KDE_RPATH = -rpath $(kde_libraries) -rpath $(qt_libraries)
LD = /usr/ccs/bin/ld
LIBCOMPAT = 
LIBCRYPT = -lcrypt
LIBDL = @LIBDL@
LIBJPEG = @LIBJPEG@
LIBOBJS = @LIBOBJS@
LIBSOCKET = -lsocket  -lnsl
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBUCB = -lucb
LN_S = ln -s
MAKEINFO = makeinfo
MOC = /home/kulow/prod/qt/bin/moc
MSGFMT = /home/kulow/bin/msgfmt
NM = /usr/ccs/bin/nm -p
PACKAGE = kdenonbeta
PAMINC = @PAMINC@
PAMLIBPATHS = @PAMLIBPATHS@
PAMLIBS = @PAMLIBS@
QT_INCLUDES = -I/home/kulow/prod/qt/include
QT_LDFLAGS = -L/home/kulow/prod/qt/lib
RANLIB = ranlib
USE_NLS = yes
VERSION = 0.1
XGETTEXT = /home/kulow/bin/xgettext
XPMINC = @XPMINC@
XPMLIB = @XPMLIB@
X_EXTRA_LIBS = -lsocket  -lnsl
X_INCLUDES = -I/usr/openwin/include
X_LDFLAGS = 
all_includes = -I/home/kulow/prod/KDE/include -I/home/kulow/prod/qt/include -I/usr/openwin/include
all_libraries = -L/home/kulow/prod/KDE/lib -L/home/kulow/prod/qt/lib 
install_root = 
kde_appsdir = /home/kulow/prod/KDE/share/applnk
kde_bindir = /home/kulow/prod/KDE/bin
kde_cgidir = /home/kulow/prod/KDE/cgi-bin
kde_confdir = /home/kulow/prod/KDE/share/config
kde_datadir = /home/kulow/prod/KDE/share/apps
kde_htmldir = /home/kulow/prod/KDE/share/doc/HTML
kde_icondir = /home/kulow/prod/KDE/share/icons
kde_includes = /home/kulow/prod/KDE/include
kde_libraries = /home/kulow/prod/KDE/lib
kde_locale = /home/kulow/prod/KDE/share/locale
kde_mimedir = /home/kulow/prod/KDE/share/mimelnk
kde_minidir = /home/kulow/prod/KDE/share/icons/mini
kde_partsdir = /home/kulow/prod/KDE/parts
kde_sounddir = /home/kulow/prod/KDE/share/sounds
kde_toolbardir = /home/kulow/prod/KDE/share/toolbar
kde_wallpaperdir = /home/kulow/prod/KDE/share/wallpapers
qt_includes = /home/kulow/prod/qt/include
qt_libraries = /home/kulow/prod/qt/lib
x_includes = /usr/openwin/include
x_libraries = /usr/lib

data_DATA = top.kdelnk

datadir = $(kde_datadir)/ktop/example
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_CLEAN_FILES = 
DATA =  $(data_DATA)

DIST_COMMON =  README Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP = --best
all: Makefile $(DATA)

.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4)
	cd $(top_srcdir) && $(AUTOMAKE) --gnu ./ktop/example/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


install-dataDATA: $(data_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(datadir)
	@list='$(data_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(datadir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(datadir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(datadir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(datadir)/$$p; \
	  fi; fi; \
	done

uninstall-dataDATA:
	@$(NORMAL_UNINSTALL)
	list='$(data_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(datadir)/$$p; \
	done
tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = ./ktop/example

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu ./ktop/example/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  test -f $(distdir)/$$file \
	  || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	  || cp -p $$d/$$file $(distdir)/$$file; \
	done
info:
dvi:
check: all
	$(MAKE)
installcheck:
install-exec: 
	@$(NORMAL_INSTALL)

install-data: install-dataDATA
	@$(NORMAL_INSTALL)

install: install-exec install-data all
	@:

uninstall: uninstall-dataDATA

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' INSTALL_SCRIPT='$(INSTALL_PROGRAM)' install
installdirs:
	$(mkinstalldirs)  $(DATADIR)$(datadir)


mostlyclean-generic:
	-test -z "$(MOSTLYCLEANFILES)" || rm -f $(MOSTLYCLEANFILES)

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(DISTCLEANFILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*
	-test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)

maintainer-clean-generic:
	-test -z "$(MAINTAINERCLEANFILES)" || rm -f $(MAINTAINERCLEANFILES)
	-test -z "$(BUILT_SOURCES)" || rm -f $(BUILT_SOURCES)
mostlyclean:  mostlyclean-generic

clean:  clean-generic mostlyclean

distclean:  distclean-generic clean
	-rm -f config.status
	-rm -f libtool

maintainer-clean:  maintainer-clean-generic distclean
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

.PHONY: uninstall-dataDATA install-dataDATA tags distdir info dvi \
installcheck install-exec install-data install uninstall all \
installdirs mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
