
# set the include path for X, qt and KDE
INCLUDES= -I$(srcdir)/.. $(all_includes)
 
lib_LTLIBRARIES = libksgrd.la

libksgrd_la_LDFLAGS = -no-undefined $(all_libraries)
libksgrd_la_LIBADD = $(LIB_KDEUI)

# Which sources should be compiled for ksysguard.
libksgrd_la_SOURCES = \
	ColorPicker.cc \
	SensorAgent.cc \
	SensorDisplay.cc \
	SensorManager.cc \
	SensorShellAgent.cc \
	SensorSocketAgent.cc \
	StyleEngine.cc \
	HostConnector.ui \
	StyleSettings.ui \
	TimerSettings.ui

ksgrdincludedir = $(includedir)/ksgrd
ksgrdinclude_HEADERS = SensorDisplay.h SensorClient.h SensorManager.h \
	SensorAgent.h SensorShellAgent.h SensorSocketAgent.h \
	StyleEngine.h ColorPicker.h

# just to make sure, automake makes them 
libksgrd_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.cc -o $(podir)/ksysguard.pot
