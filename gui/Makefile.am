# just set the variable
appsdir = $(kde_appsdir)/System
# set the include path for X, qt and KDE
INCLUDES= $(all_includes)
# claim, which subdirectories you want to install
SUBDIRS =
 
####### This part is very ktop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	ktop

# Which sources should be compiled for ktop.

ktop_SOURCES = ktop.cc  SensorAgent.cc SensorManager.cc SensorBrowser.cc \
	Workspace.cc MainMenu.cc ProcessMenu.cc ReniceDlg.cc FancyPlotter.cc \
	SignalPlotter.cc MultiMeter.cc

#	TaskMan.cc \
#	ProcListPage.cc ProcessList.cc \
#	OSProcessList.cc OSProcess.cc PerfMonPage.cc \
#	FancyPlotter.cc MultiMeter.cc SignalPlotter.cc OSStatus.cc \

# the library search path
ktop_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
#ktop_LDADD   =       $(LIB_KDEUI) $(LIBKVM)
ktop_LDADD   =       $(LIB_KDEUI)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = MainMenu.h ReniceDlg.h SensorClient.h ktop.h ProcessMenu.h \
	Workspace.h SensorAgent.h SensorManager.h SensorBrowser.h version.h \
	FancyPlotter.h SignalPlotter.h MultiMeter.h

# just to make sure, automake makes them 

METASOURCES = ktop.moc SensorAgent.moc SensorManager.moc SensorBrowser.moc MainMenu.moc ProcessMenu.moc ReniceDlg.moc Workspace.moc FancyPlotter.moc SignalPlotter.moc MultiMeter.moc
# TaskMan.moc ProcListPage.moc ProcessList.moc PerfMonPage.moc FancyPlotter.moc MultiMeter.moc SignalPlotter.moc

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(ktop_SOURCES) && mv messages.po ../po/ktop.pot

KDE_ICON = ktop

apps_DATA = ktop.desktop

