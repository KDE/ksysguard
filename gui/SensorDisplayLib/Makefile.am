
# set the include path for X, qt and KDE
INCLUDES= -I$(srcdir)/.. -I$(srcdir)/../ksgrd -I../ksgrd $(all_includes)
 
lib_LTLIBRARIES = libsensordisplays.la

libsensordisplays_la_LDFLAGS = $(all_libraries)
libsensordisplays_la_LIBADD = $(LIB_KDEUI)

# Which sources should be compiled for the sensor display lib.
libsensordisplays_la_SOURCES = \
	SensorDisplay.cc \
	BarGraph.cc \
	DancingBars.cc \
	DummyDisplay.cc \
	FancyPlotter.cc \
	ListView.cc \
	LogFile.cc \
	MultiMeter.cc \
	ProcessController.cc \
	ProcessList.cc \
	ReniceDlg.cc \
	SensorLogger.cc \
	SignalPlotter.cc \
	BarGraphSettings.ui \
	DancingBarsSettings.ui \
	FancyPlotterSettings.ui \
	ListViewSettings.ui \
	LogFileSettings.ui \
	MultiMeterSettings.ui \
	SensorLoggerDlg.ui \
	SensorLoggerSettings.ui

libsensordisplayincludedir = $(includedir)/SensorDisplayLib
#libsensordisplayinclude_HEADERS = 
#	SensorDisplay.h 
#	BarGraph.h 
#	BarGraphSettings.h 
#	DancingBars.h 
#	DancingBarsSettings.h 
#	DummyDisplay.h 
#	FancyPlotter.h
#	FancyPlotterSettings.h 
#	ListView.h 
#	ListViewSettings.h 
#	LogFile.h 
#	LogFileSettings.h 
#	MultiMeter.h 
#	MultiMeterSettings.h 
#	ProcessController.h 
#	ProcessList.h 
#	ReniceDlg.h 
#	SensorLogger.h 
#	SensorLoggerDlg.h 
#	SensorLoggerSettings.h 
#	SignalPlotter.h

# just to make sure, automake makes them 
METASOURCES = AUTO
