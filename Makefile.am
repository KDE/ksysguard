
# just set the variable
appsdir = $(kde_appsdir)/System
# set the include path for X, qt and KDE
INCLUDES= $(all_includes)
# claim, which subdirectories you want to install
SUBDIRS = CContLib ktopd pics
 
####### This part is very ktop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	ktop

# Which sources should be compiled for ktop.

ktop_SOURCES = ktop.cpp  SensorAgent.cpp SensorManager.cpp \
	MainMenu.cpp ProcessMenu.cpp \
	TaskMan.cpp \
	ProcListPage.cpp ProcessList.cpp \
	OSProcessList.cpp OSProcess.cpp PerfMonPage.cpp \
	FancyPlotter.cpp MultiMeter.cpp SignalPlotter.cpp OSStatus.cpp \
	ReniceDlg.cpp

# the library search path
ktop_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
#ktop_LDADD   =       $(LIB_KDEUI) $(LIBKVM)
ktop_LDADD   =       $(LIB_KDEUI)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = ktop.h SensorAgent.h SensorManager.h \
	MainMenu.h ProcessMenu.h TaskMan.h \
	ProcListPage.h \
	ProcessList.h OSProcessList.h \
	PerfMonPage.h FancyPlotter.h MultiMeter.h SignalPlotter.h OSStatus.h \
	ReniceDlg.h

# just to make sure, automake makes them 

METASOURCES = ktop.moc SensorAgent.moc SensorManager.moc MainMenu.moc ProcessMenu.moc TaskMan.moc ProcListPage.moc ProcessList.moc PerfMonPage.moc FancyPlotter.moc MultiMeter.moc SignalPlotter.moc ReniceDlg.moc

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(ktop_SOURCES) && mv messages.po ../po/ktop.pot

KDE_ICON = ktop

apps_DATA = ktop.desktop

