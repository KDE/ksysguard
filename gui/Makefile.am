# just set the variable
appsdir = $(kde_appsdir)/System
datadir = $(kde_datadir)/ktop

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)
 
####### This part is very ktop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	ktop

# Which sources should be compiled for ktop.

ktop_SOURCES = ktop.cc ktop.skel SensorAgent.cc SensorManager.cc \
	HostConnector.cc WorkSheetSetup.cc \
	ProcessMenu.cc ReniceDlg.cc WorkSheet.cc Workspace.cc \
	SensorBrowser.cc SensorDisplay.cc \
	FancyPlotter.cc SignalPlotter.cc MultiMeter.cc \
	ProcessController.cc ProcessList.cc 

# the library search path
ktop_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
ktop_LDADD   =       $(LIB_KDEUI) $(LIB_KFILE)

data_DATA = ProcessTable.ktop SystemLoad.ktop

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = SensorClient.h ktop.h ProcessMenu.h \
	Workspace.h WorkSheet.h SensorBrowser.h \
	HostConnector.h WorkSheetSetup.h \
	SensorAgent.h SensorManager.h SensorDisplay.h \
	FancyPlotter.h SignalPlotter.h MultiMeter.h \
	ProcessController.h ProcessList.h version.h 

# just to make sure, automake makes them 
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cc -o $(podir)/ktop.pot

rcdir = $(kde_datadir)/ktop
rc_DATA = ktop.rc

apps_DATA = ktop.desktop
