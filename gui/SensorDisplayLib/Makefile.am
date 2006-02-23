
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
	DancingBarsSettings.cc \
	DummyDisplay.cc \
	FancyPlotter.cc \
	FancyPlotterSettings.cc \
	ListView.cc \
	LogFile.cc \
	MultiMeter.cc \
	MultiMeterSettings.cc \
	Process.cc \
	ProcessController.cc \
	ProcessModel.cc \
	ProcessFilter.cc \
	ReniceDlg.cc \
	SensorLogger.cc \
	SensorLoggerDlg.cc \
	SensorLoggerSettings.cc \
	ListViewSettings.cc \
	SignalPlotter.cc \
	ListViewSettingsWidget.ui \
	LogFileSettings.ui \
	MultiMeterSettingsWidget.ui \
	SensorLoggerDlgWidget.ui \
	SensorLoggerSettingsWidget.ui \
	ProcessWidgetUI.ui

# just to make sure, automake makes them 
METASOURCES = AUTO

