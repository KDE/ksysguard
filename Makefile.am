
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
APPSDIR = $(kde_appsdir)/System
# set the include path for X, qt and KDE
INCLUDES= $(all_includes)
# claim, which subdirectories you want to install
SUBDIRS = pics
 
####### This part is very ktop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	ktop

# Which sources should be compiled for ktop.
ktop_SOURCES	= ktop.cpp MainMenu.cpp ProcessMenu.cpp \
	TaskMan.cpp \
	ProcListPage.cpp ProcessList.cpp \
	OSProcessList.cpp OSProcess.cpp PerfMonPage.cpp \
	FancyPlotter.cpp MultiMeter.cpp SignalPlotter.cpp OSStatus.cpp \
	ReniceDlg.cpp

# the library search path
ktop_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
#ktop_LDADD   =      $(LIB_KFM) $(LIB_KDEUI) $(LIBKVM)
ktop_LDADD   =      $(LIB_KFM) $(LIB_KDEUI)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = ktop.h MainMenu.h ProcessMenu.h TaskMan.h \
	ProcListPage.h \
	ProcessList.h OSProcessList.h \
	PerfMonPage.h FancyPlotter.h MultiMeter.h SignalPlotter.h OSStatus.h \
	ReniceDlg.h

# just to make sure, automake makes them 
METASOURCES =	ktop.moc MainMenu.moc ProcessMenu.moc TaskMan.moc ProcListPage.moc ProcessList.moc PerfMonPage.moc FancyPlotter.moc MultiMeter.moc SignalPlotter.moc ReniceDlg.moc

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(ktop_SOURCES) && mv messages.po ../po/ktop.pot

kde_icon_DATA = ktop.xpm

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) $(srcdir)/ktop.desktop $(APPSDIR)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-ktop.xpm $(kde_minidir)/ktop.xpm

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/ktop.desktop
	-rm -f $(kde_minidir)/mini-ktop.xpm
