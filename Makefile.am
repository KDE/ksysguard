
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
INCLUDES= @all_includes@
# claim, which subdirectories you want to install
SUBDIRS = doc pics example
 
####### This part is very ktop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	ktop

# Which sources should be compiled for ktop.
ktop_SOURCES	= ktop.cpp memory.cpp settings.cpp cpu.cpp widgets.cpp ptree.cpp

# the library search path
ktop_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
ktop_LDADD   = -lkfm -lkdeui -lkdecore -lqt -lX11 -lXext @LIBSOCKET@     

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = ktop.h memory.h settings.h cpu.h widgets.h

# just to make sure, automake makes them 
BUILTSOURCES =	ktop.moc memory.moc settings.moc cpu.moc widgets.moc

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES)

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




