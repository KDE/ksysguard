# just set the variable
appsdir = $(kde_appsdir)/System
datadir = $(kde_datadir)/ksysguard

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)
 
####### This part is very ksysguard specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	ksysguard

# Which sources should be compiled for ksysguard.
ksysguard_SOURCES = ksysguard.cc ksysguard.skel SensorAgent.cc \
	SensorManager.cc WorkSheetSetup.cc \
	ProcessMenu.cc ReniceDlg.cc WorkSheet.cc Workspace.cc \
	SensorBrowser.cc SensorDisplay.cc \
	FancyPlotter.cc SignalPlotter.cc MultiMeter.cc \
	DancingBars.cc BarGraph.cc \
	ProcessController.cc ProcessList.cc \
	DancingBarsSettings.ui MultiMeterSettings.ui \
	FancyPlotterSettings.ui HostConnector.ui

# the library search path
ksysguard_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
ksysguard_LDADD   =       $(LIB_KDEUI) $(LIB_KFILE)

data_DATA = ProcessTable.sgrd SystemLoad.sgrd

rcdir = $(kde_datadir)/ksysguard
rc_DATA = ksysguardui.rc

apps_DATA = ksysguard.desktop

# This stuff is now for the kicker applet
lib_LTLIBRARIES = libksysguardapplet.la

libksysguardapplet_la_SOURCES = KSysGuardApplet.cc SensorManager.cc \
	SensorAgent.cc FancyPlotter.cc SignalPlotter.cc SensorDisplay.cc \
	FancyPlotterSettings.ui
libksysguardapplet_la_METASOURCES = AUTO
libksysguardapplet_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module -no-undefined
libksysguardapplet_la_LIBADD = $(LIB_KDEUI) $(LIB_KSYCOCA)

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = ksysguardapplet.desktop
EXTRA_DIST = $(lnk_DATA)

# just to make sure, automake makes them 
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.cc -o $(podir)/ksysguard.pot

