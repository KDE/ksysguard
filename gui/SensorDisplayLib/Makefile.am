
# set the include path for X, qt and KDE
INCLUDES= -I$(srcdir)/.. -I$(srcdir)/../ksgrd -I../ksgrd $(all_includes)
 
noinst_LTLIBRARIES = libsensordisplays.la

libsensordisplays_la_LDFLAGS = $(all_libraries) -no-undefined
libsensordisplays_la_LIBADD = ../ksgrd/libksgrd.la $(LIB_KIO)

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
	DancingBarsSettings.ui \
	FancyPlotterSettings.ui \
	ListViewSettings.ui \
	LogFileSettings.ui \
	MultiMeterSettings.ui \
	SensorLoggerDlg.ui \
	SensorLoggerSettings.ui

# just to make sure, automake makes them 
METASOURCES = AUTO

SensorDisplay.lo: ../ksgrd/TimerSettings.h
