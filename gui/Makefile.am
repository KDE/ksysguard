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
	SensorManager.cc HostConnector.cc WorkSheetSetup.cc \
	ProcessMenu.cc ReniceDlg.cc WorkSheet.cc Workspace.cc \
	SensorBrowser.cc SensorDisplay.cc \
	FancyPlotter.cc SignalPlotter.cc MultiMeter.cc \
	MultiMeterSettings.ui DancingBars.cc BarGraph.cc \
	ProcessController.cc ProcessList.cc

# the library search path
ksysguard_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
ksysguard_LDADD   =       $(LIB_KDEUI) $(LIB_KFILE)

data_DATA = ProcessTable.sgrd SystemLoad.sgrd

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = SensorClient.h ksysguard.h ProcessMenu.h \
	Workspace.h WorkSheet.h SensorBrowser.h DancingBars.h \
	BarGraph.h HostConnector.h WorkSheetSetup.h \
	SensorAgent.h SensorManager.h SensorDisplay.h \
	FancyPlotter.h SignalPlotter.h MultiMeter.h \
	MultiMeterSettings.h \
	ProcessController.h ProcessList.h version.h 

# just to make sure, automake makes them 
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.cc -o $(podir)/ksysguard.pot

rcdir = $(kde_datadir)/ksysguard
rc_DATA = ksysguardui.rc

apps_DATA = ksysguard.desktop
