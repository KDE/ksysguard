
# set the include path for X, qt and KDE
INCLUDES= -I$(srcdir)/../SensorDisplayLib $(all_includes)
 
lib_LTLIBRARIES = libksgrd.la

libksgrd_la_LDFLAGS = -no-undefined -version-info 3:0:2 $(all_libraries)
libksgrd_la_LIBADD = $(LIB_KDEUI)

# Which sources should be compiled for ksysguard.
libksgrd_la_SOURCES = \
	ColorPicker.cc \
	SensorAgent.cc \
	SensorManager.cc \
	SensorShellAgent.cc \
	SensorSocketAgent.cc \
	StyleEngine.cc \
	StyleSettings.cc \
	TimerSettings.cc \
	HostConnector.ui

ksgrdincludedir = $(includedir)/ksgrd
ksgrdinclude_HEADERS = \
	ColorPicker.h \
	HostConnector.h \
	SensorAgent.h \
	SensorClient.h \
	SensorManager.h \
	SensorShellAgent.h \
	SensorSocketAgent.h \
	StyleEngine.h \
	StyleSettings.h \
	TimerSettings.h

# just to make sure, automake makes them 
libksgrd_la_METASOURCES = AUTO
