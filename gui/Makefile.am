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
SUBDIRS = ksgrd

# set the include path for X, qt and KDE
INCLUDES = $(all_includes)
 
####### This part is very ksysguard specific
# you can add here more. This one gets installed 
bin_PROGRAMS = ksysguard kpm

# Which sources should be compiled for ksysguard.
ksysguard_SOURCES = \
	BarGraph.cc \
	DancingBars.cc \
	DummyDisplay.cc \
	FancyPlotter.cc \
	ksysguard.cc ksysguard.skel \
	ListView.cc \
	LogFile.cc \
	MultiMeter.cc \
	ProcessController.cc \
	ProcessList.cc \
	ReniceDlg.cc \
	SensorBrowser.cc  \
	SensorLogger.cc \
	SignalPlotter.cc \
	WorkSheet.cc \
	Workspace.cc \
	BarGraphSettings.ui \
	DancingBarsSettings.ui \
	FancyPlotterSettings.ui \
	ListViewSettings.ui \
	LogFileSettings.ui \
	MultiMeterSettings.ui \
	SensorLoggerDlg.ui \
	SensorLoggerSettings.ui \
	WorkSheetSettings.ui


ksysguard_LDADD   = ksgrd/libksgrd.la $(LIB_KDEUI) $(LIB_KFILE)
ksysguard_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kpm_SOURCES = kpm.c

data_DATA = ProcessTable.sgrd SystemLoad.sgrd KSysGuardApplet.xml

# This stuff is now for the kicker applet
kde_module_LTLIBRARIES = sysguard_panelapplet.la

sysguard_panelapplet_la_SOURCES = \
	BarGraph.cc \
	DancingBars.cc \
	FancyPlotter.cc \
	KSysGuardApplet.cc \
	MultiMeter.cc \
	SignalPlotter.cc \
	BarGraphSettings.ui \
	DancingBarsSettings.ui \
	FancyPlotterSettings.ui \
	KSysGuardAppletSettings.ui \
	MultiMeterSettings.ui

sysguard_panelapplet_la_METASOURCES = AUTO
sysguard_panelapplet_la_LIBADD   = ksgrd/libksgrd.la $(LIB_KDEUI) $(LIB_KFILE)
sysguard_panelapplet_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module -no-undefined

EXTRA_DIST = $(lnk_DATA)

# just to make sure, automake makes them 
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) ksgrd/*.ui >> rc.cpp
	$(XGETTEXT) ksgrd/*.cpp ksgrd/*.cc *.cpp *.cc -o $(podir)/ksysguard.pot
