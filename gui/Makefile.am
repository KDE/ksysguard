# just set the variable
appsdir = $(kde_appsdir)/System
datadir = $(kde_datadir)/ksysguard

kdemime_DATA = x-ksysguard.desktop
kdemimedir = $(kde_mimedir)/application

rcdir = $(kde_datadir)/ksysguard
rc_DATA = ksysguardui.rc

apps_DATA = ksysguard.desktop

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = ksysguardapplet.desktop

# claim, which subdirectories you want to install
SUBDIRS = ksgrd SensorDisplayLib

# set the include path for X, qt and KDE
INCLUDES = -I$(srcdir)/ksgrd -I$(srcdir)/SensorDisplayLib -I$(top_builddir)/ksysguard/gui/SensorDisplayLib $(all_includes)
 
####### This part is very ksysguard specific
# you can add here more. This one gets installed 
bin_PROGRAMS = ksysguard kpm

# Which sources should be compiled for ksysguard.
ksysguard_SOURCES = \
	SensorBrowser.cc \
	WorkSheet.cc \
	WorkSheetSettings.ui \
	Workspace.cc \
	ksysguard.cc ksysguard.skel

ksysguard_LDADD = \
	ksgrd/libksgrd.la \
	SensorDisplayLib/libsensordisplays.la \
	$(LIB_KDEUI) $(LIB_KFILE)
ksysguard_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kpm_SOURCES = kpm.c

data_DATA = ProcessTable.sgrd SystemLoad.sgrd KSysGuardApplet.xml

# This stuff is now for the kicker applet
kde_module_LTLIBRARIES = sysguard_panelapplet.la

sysguard_panelapplet_la_SOURCES = \
	KSysGuardApplet.cc \
	KSysGuardAppletSettings.ui

sysguard_panelapplet_la_LIBADD = \
	ksgrd/libksgrd.la \
	SensorDisplayLib/libsensordisplays.la \
	$(LIB_KDEUI) $(LIB_KFILE)
sysguard_panelapplet_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module -no-undefined

EXTRA_DIST = $(lnk_DATA)

# just to make sure, automake makes them 
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) ksgrd/*.ui >> rc.cpp
	$(XGETTEXT) ksgrd/*.cc *.cpp *.cc -o $(podir)/ksysguard.pot
