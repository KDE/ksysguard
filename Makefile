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

# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# just set the variable


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /home/kulow/prod/KDE
exec_prefix = ${prefix}

bindir = /home/kulow/prod/KDE/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
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

top_builddir = ..

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

APPSDIR = $(kde_appsdir)/System
# set the include path for X, qt and KDE
INCLUDES= -I/home/kulow/prod/KDE/include -I/home/kulow/prod/qt/include -I/usr/openwin/include
# claim, which subdirectories you want to install
SUBDIRS = doc pics example

####### This part is very ktop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	ktop

# Which sources should be compiled for ktop.
ktop_SOURCES	= ktop.cpp memory.cpp settings.cpp cpu.cpp widgets.cpp ptree.cpp

# the library search path
ktop_LDFLAGS = -L/home/kulow/prod/KDE/lib -L/home/kulow/prod/qt/lib 

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
ktop_LDADD   = -lkfm -lkdeui -lkdecore -lqt -lX11 -lXext -lsocket  -lnsl     

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = ktop.h memory.h settings.h cpu.h widgets.h

# just to make sure, automake makes them 
BUILTSOURCES =	ktop.moc memory.moc settings.moc cpu.moc widgets.moc

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES)
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_CLEAN_FILES = 
PROGRAMS =  $(bin_PROGRAMS)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) 
CPPFLAGS = 
LDFLAGS = 
LIBS = 
ktop_OBJECTS =  ktop.o memory.o settings.o cpu.o widgets.o ptree.o
ktop_DEPENDENCIES = 
CXXFLAGS = -g -Wall
CXXCOMPILE = $(CXX) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CXXFLAGS)
LTCXXCOMPILE = $(LIBTOOL) --mode=compile $(CXX) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CXXFLAGS)
CXXLINK = $(LIBTOOL) --mode=link $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@
HEADERS =  $(noinst_HEADERS)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP = --best
DEP_FILES =  .deps/cpu.P .deps/ktop.P .deps/memory.P .deps/ptree.P \
.deps/settings.P .deps/widgets.P
CXXMKDEP = $(CXX) -M $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CXXFLAGS)
SOURCES = $(ktop_SOURCES)
OBJECTS = $(ktop_OBJECTS)

all: all-recursive all-am

.SUFFIXES:
.SUFFIXES: .S .c .cpp .lo .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4)
	cd $(top_srcdir) && $(AUTOMAKE) --gnu ./ktop/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`"; \
	    $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`; \
	done

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

.s.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

.S.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

mostlyclean-libtool:
	-rm -f *.lo

clean-libtool:
	-rm -rf .libs _libs

distclean-libtool:

maintainer-clean-libtool:

ktop: $(ktop_OBJECTS) $(ktop_DEPENDENCIES)
	@rm -f ktop
	$(CXXLINK) $(ktop_LDFLAGS) $(ktop_OBJECTS) $(ktop_LDADD) $(LIBS)
.cpp.o:
	$(CXXCOMPILE) -c $<
.cpp.lo:
	$(LTCXXCOMPILE) -c $<

# This directory's subdirectories are mostly independent; you can cd
# into them and run `make' without going through this Makefile.
# To change the values of `make' variables: instead of editing Makefiles,
# (1) if the variable is set in `config.status', edit `config.status'
#     (which will cause the Makefiles to be regenerated when you run `make');
# (2) otherwise, pass the desired values on the `make' command line.



all-recursive install-data-recursive install-exec-recursive \
installdirs-recursive install-recursive uninstall-recursive  \
check-recursive installcheck-recursive info-recursive dvi-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  target=`echo $@ | sed s/-recursive//`; \
	  echo "Making $$target in $$subdir"; \
	  (cd $$subdir && $(MAKE) $$target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"

mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	rev=''; list='$(SUBDIRS)'; for subdir in $$list; do \
	  rev="$$subdir $$rev"; \
	done; \
	for subdir in $$rev; do \
	  target=`echo $@ | sed s/-recursive//`; \
	  echo "Making $$target in $$subdir"; \
	  (cd $$subdir && $(MAKE) $$target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
tags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  (cd $$subdir && $(MAKE) tags); \
	done

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $(SOURCES) $(HEADERS) $(LISP)

TAGS: tags-recursive $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test -f $$subdir/TAGS && tags="$$tags -i $$here/$$subdir/TAGS"; \
	done; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = ./ktop

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu ./ktop/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  test -f $(distdir)/$$file \
	  || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	  || cp -p $$d/$$file $(distdir)/$$file; \
	done
	for subdir in $(SUBDIRS); do \
	  test -d $(distdir)/$$subdir \
	  || mkdir $(distdir)/$$subdir \
	  || exit 1; \
	  chmod 777 $(distdir)/$$subdir; \
	  (cd $$subdir && $(MAKE) top_distdir=../$(top_distdir) distdir=../$(distdir)/$$subdir distdir) \
	    || exit 1; \
	done

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

-include $(DEP_FILES)

mostlyclean-depend:

clean-depend:

distclean-depend:

maintainer-clean-depend:
	-rm -rf .deps

%.o: %.c
	@echo '$(COMPILE) -c $<'; \
	$(COMPILE) -Wp,-MD,.deps/$(*F).P -c $<

%.lo: %.c
	@echo '$(LTCOMPILE) -c $<'; \
	$(LTCOMPILE) -Wp,-MD,.deps/$(*F).p -c $<
	@-sed -e 's/^\([^:]*\)\.o:/\1.lo \1.o:/' \
	  < .deps/$(*F).p > .deps/$(*F).P
	@-rm -f .deps/$(*F).p

%.o: %.cpp
	@echo '$(CXXCOMPILE) -c $<'; \
	$(CXXCOMPILE) -Wp,-MD,.deps/$(*F).P -c $<

%.lo: %.cpp
	@echo '$(LTCXXCOMPILE) -c $<'; \
	$(LTCXXCOMPILE) -Wp,-MD,.deps/$(*F).p -c $<
	@-sed -e 's/^\([^:]*\)\.o:/\1.lo \1.o:/' \
	  < .deps/$(*F).p > .deps/$(*F).P
	@-rm -f .deps/$(*F).p
info: info-recursive
dvi: dvi-recursive
check: all-am
	$(MAKE) check-recursive
installcheck: installcheck-recursive
all-am: Makefile $(PROGRAMS) $(HEADERS)

install-exec-am: install-binPROGRAMS

install-data-am: install-data-local

uninstall-am: uninstall-binPROGRAMS uninstall-local

install-exec: install-exec-recursive install-exec-am
	@$(NORMAL_INSTALL)

install-data: install-data-recursive install-data-am
	@$(NORMAL_INSTALL)

install: install-recursive install-exec-am install-data-am
	@:

uninstall: uninstall-recursive uninstall-am

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' INSTALL_SCRIPT='$(INSTALL_PROGRAM)' install
installdirs: installdirs-recursive
	$(mkinstalldirs)  $(DATADIR)$(bindir)


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
mostlyclean-am:  mostlyclean-binPROGRAMS mostlyclean-compile \
		mostlyclean-libtool mostlyclean-tags mostlyclean-depend \
		mostlyclean-generic

clean-am:  clean-binPROGRAMS clean-compile clean-libtool clean-tags \
		clean-depend clean-generic mostlyclean-am

distclean-am:  distclean-binPROGRAMS distclean-compile distclean-libtool \
		distclean-tags distclean-depend distclean-generic \
		clean-am

maintainer-clean-am:  maintainer-clean-binPROGRAMS \
		maintainer-clean-compile maintainer-clean-libtool \
		maintainer-clean-tags maintainer-clean-depend \
		maintainer-clean-generic distclean-am

mostlyclean:  mostlyclean-recursive mostlyclean-am

clean:  clean-recursive clean-am

distclean:  distclean-recursive distclean-am
	-rm -f config.status
	-rm -f libtool

maintainer-clean:  maintainer-clean-recursive maintainer-clean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

.PHONY: mostlyclean-binPROGRAMS distclean-binPROGRAMS clean-binPROGRAMS \
maintainer-clean-binPROGRAMS uninstall-binPROGRAMS install-binPROGRAMS \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile mostlyclean-libtool distclean-libtool \
clean-libtool maintainer-clean-libtool install-data-recursive \
uninstall-data-recursive install-exec-recursive \
uninstall-exec-recursive installdirs-recursive uninstalldirs-recursive \
all-recursive check-recursive installcheck-recursive info-recursive \
dvi-recursive mostlyclean-recursive distclean-recursive clean-recursive \
maintainer-clean-recursive tags tags-recursive mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir \
mostlyclean-depend distclean-depend clean-depend \
maintainer-clean-depend info dvi installcheck all-am install-exec-am \
install-data-am uninstall-am install-exec install-data install \
uninstall all installdirs mostlyclean-generic distclean-generic \
clean-generic maintainer-clean-generic clean mostlyclean distclean \
maintainer-clean


# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate -kktr  $(ktop_SOURCES)

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) ktop.kdelnk $(APPSDIR)
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) ktop.xpm $(kde_icondir)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-ktop.xpm $(kde_minidir)/mini-ktop.xpm

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/ktop.kdelnk
	-rm -f $(kde_icondir)/ktop.xpm
	-rm -f $(kde_minidir)/mini-ktop.xpm

# add a dependency for every moc file to be full portable
# I've added a key binding to emacs for this. 
ktop.cpp:ktop.moc
ktop.moc: ktop.h
	$(MOC) ktop.h -o ktop.moc

memory.cpp:memory.moc
memory.moc: memory.h
	$(MOC) memory.h -o memory.moc

cpu.cpp:cpu.moc
cpu.moc: cpu.h
	$(MOC) cpu.h -o cpu.moc

widgets.cpp:widgets.moc
widgets.moc: widgets.h
	$(MOC) widgets.h -o widgets.moc

settings.cpp:settings.moc
settings.moc: settings.h
	$(MOC) settings.h -o settings.moc

ptree.cpp:ptree.moc
ptree.moc: ptree.h
	$(MOC) ptree.h -o ptree.moc	

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
