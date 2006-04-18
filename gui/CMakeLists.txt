kde4_header()

add_subdirectory( ksgrd ) 
add_subdirectory( SensorDisplayLib ) 
include_directories( ${CMAKE_SOURCE_DIR}/workspace/kicker/libkicker/ ${CMAKE_SOURCE_DIR}/kicker/libkicker/  ${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/ksgrd/ ${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/ ${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/ ${KDE4_INCLUDES} )


########### next target ###############

set(libsensordisplays_SRCS
	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/SensorDisplay.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/BarGraph.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/DancingBars.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/DancingBarsSettings.cc 
	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/DummyDisplay.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/FancyPlotter.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/FancyPlotterSettings.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/ListView.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/LogFile.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/MultiMeter.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/MultiMeterSettings.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/Process.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/ProcessController.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/ProcessModel.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/ProcessFilter.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/ReniceDlg.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/SensorLogger.cc
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/SensorLoggerDlg.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/SensorLoggerSettings.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/ListViewSettings.cc 
   	${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/SignalPlotter.cc 
	)
kde4_add_ui3_files( libsensordisplays_SRCS
   ${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/ListViewSettingsWidget.ui 
   ${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/LogFileSettings.ui 
   ${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/MultiMeterSettingsWidget.ui 
   ${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/SensorLoggerDlgWidget.ui 
   ${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/SensorLoggerSettingsWidget.ui )

kde4_add_ui_files( libsensordisplays_SRCS ${CMAKE_SOURCE_DIR}/workspace/ksysguard/gui/SensorDisplayLib/ProcessWidgetUI.ui )


set(ksysguard_SRCS ${libsensordisplays_SRCS}
   SensorBrowser.cc 
   WorkSheet.cc
   SensorFrame.cc
   WorkSheetSettings.cc 
   Workspace.cc 
   ksysguard.cc )

kde4_automoc(${ksysguard_SRCS})

kde4_add_dcop_skels(ksysguard_SRCS ksysguard.h )

kde4_add_executable(ksysguard ${ksysguard_SRCS})

target_link_libraries(ksysguard  ${KDE4_KIO_LIBS} ksgrd  )

install_targets(/bin ksysguard )


########### next target ###############

set(kpm_SRCS kpm.c )

kde4_automoc(${kpm_SRCS})

kde4_add_executable(kpm ${kpm_SRCS})

target_link_libraries(kpm  ${KDE4_KDECORE_LIBS} )

install_targets(/bin kpm )


########### next target ###############

set(sysguard_panelapplet_PART_SRCS KSysGuardApplet.cc KSGAppletSettings.cc ${libsensordisplays_SRCS} )

kde4_automoc(${sysguard_panelapplet_PART_SRCS})

kde4_add_plugin(sysguard_panelapplet ${sysguard_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} sysguard_panelapplet )

target_link_libraries(sysguard_panelapplet  ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} ksgrd  kickermain  )

install_targets(${PLUGIN_INSTALL_DIR} sysguard_panelapplet )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES ksysguard.desktop )
install_files( ${DATA_INSTALL_DIR}/ksysguard FILES ProcessTable.sgrd SystemLoad.sgrd KSysGuardApplet.xml )
install_files( ${MIME_INSTALL_DIR}/application FILES x-ksysguard.desktop )
install_files( ${DATA_INSTALL_DIR}/ksysguard FILES ksysguardui.rc )
install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES ksysguardapplet.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#kdemimedir = $(kde_mimedir)/application
#kdemime_DATA = x-ksysguard.desktop
#
#rcdir = $(kde_datadir)/ksysguard
#rc_DATA = ksysguardui.rc
#
#xdg_apps_DATA = ksysguard.desktop
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = ksysguardapplet.desktop
#
## claim, which subdirectories you want to install
#SUBDIRS = ksgrd SensorDisplayLib
#
## set the include path for X, qt and KDE
#INCLUDES = -I$(srcdir)/ksgrd -I$(srcdir)/SensorDisplayLib -ISensorDisplayLib -I$(top_srcdir)/kicker/libkicker/ $(all_includes)
# 
######## This part is very ksysguard specific
## you can add here more. This one gets installed 
#bin_PROGRAMS = ksysguard kpm
#
## Which sources should be compiled for ksysguard.
#ksysguard_SOURCES = \
#	SensorBrowser.cc \
#	WorkSheet.cc \
#	WorkSheetSettings.cc \
#	Workspace.cc \
#	ksysguard.cc ksysguard.skel
#
#ksysguard_LDADD = \
#	ksgrd/libksgrd.la \
#	SensorDisplayLib/libsensordisplays.la \
#	$(LIB_KDEUI) $(LIB_KIO)
#ksysguard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#kpm_SOURCES = kpm.c
#
#appdatadir = $(kde_datadir)/ksysguard
#appdata_DATA = ProcessTable.sgrd SystemLoad.sgrd KSysGuardApplet.xml
#
## This stuff is now for the kicker applet
#kde_module_LTLIBRARIES = sysguard_panelapplet.la
#
#sysguard_panelapplet_la_SOURCES = \
#	KSysGuardApplet.cc \
#	KSGAppletSettings.cc
#
#sysguard_panelapplet_la_LIBADD = \
#	ksgrd/libksgrd.la \
#	SensorDisplayLib/libsensordisplays.la \
#	$(top_builddir)/kicker/libkicker/libkickermain.la $(LIB_KDEUI) $(LIB_KIO)
#sysguard_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#
#EXTRA_DIST = $(lnk_DATA)
#
## just to make sure, automake makes them 
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(EXTRACTRC) `find . -name "*.ui"` >> rc.cpp
#	$(EXTRACTRC) `find . -name "*.sgrd"` >> rc.cpp
#	$(XGETTEXT) `find . -name "*.cpp" -o -name "*.cc"` -o $(podir)/ksysguard.pot
