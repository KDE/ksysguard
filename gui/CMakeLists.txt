
add_subdirectory( ksgrd ) 
add_subdirectory( SensorDisplayLib ) 

include_directories( ${CMAKE_SOURCE_DIR}/workspace/kicker/libkicker/ ${CMAKE_SOURCE_DIR}/kicker/libkicker/  ${CMAKE_CURRENT_SOURCE_DIR}/ksgrd/ ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ )


########### next target ###############

set(libsensordisplays_SRCS
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorDisplay.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/BarGraph.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/DancingBars.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/DancingBarsSettings.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/DummyDisplay.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/FancyPlotter.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/FancyPlotterSettings.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ListView.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/LogFile.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/MultiMeter.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/MultiMeterSettings.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/Process.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ProcessController.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ProcessModel.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ProcessFilter.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ReniceDlg.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLogger.cc
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerDlg.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerSettings.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorModel.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ListViewSettings.cc 
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SignalPlotter.cc
#   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/modeltest.cpp
	)

kde4_add_ui_files( libsensordisplays_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ListViewSettingsWidget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/LogFileSettings.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/MultiMeterSettingsWidget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ProcessWidgetUI.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerDlgWidget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerSettingsWidget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ReniceDlgUi.ui
)

set(ksysguard_KDEINIT_SRCS ${libsensordisplays_SRCS}
   SensorBrowser.cc 
   WorkSheet.cc
   SensorFrame.cc
   WorkSheetSettings.cc 
   Workspace.cc
   HostConnector.cc
   StyleEngine.cc
   StyleSettings.cc
   TimerSettings.cc 
   ksysguard.cc )

kde4_automoc(${ksysguard_KDEINIT_SRCS})


kde4_add_kdeinit_executable(ksysguard ${ksysguard_KDEINIT_SRCS})

target_link_libraries(kdeinit_ksysguard  ${KDE4_KIO_LIBS} ksgrd  )

install(TARGETS kdeinit_ksysguard DESTINATION ${LIB_INSTALL_DIR})

target_link_libraries(ksysguard kdeinit_ksysguard)

install(TARGETS ksysguard DESTINATION ${BIN_INSTALL_DIR})
########### next target ###############

set(sysguard_panelapplet_PART_SRCS KSysGuardApplet.cc KSGAppletSettings.cc ${libsensordisplays_SRCS} StyleEngine.cc StyleSettings.cc TimerSettings.cc)

kde4_automoc(${sysguard_panelapplet_PART_SRCS})

kde4_add_plugin(sysguard_panelapplet ${sysguard_panelapplet_PART_SRCS})


target_link_libraries(sysguard_panelapplet  ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} ksgrd  kickermain  )

install(TARGETS sysguard_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ksysguard.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES ProcessTable.sgrd SystemLoad.sgrd KSysGuardApplet.xml default_theme.svg DESTINATION  ${DATA_INSTALL_DIR}/ksysguard )
install( FILES x-ksysguard.desktop  DESTINATION  ${MIME_INSTALL_DIR}/application )
install( FILES ksysguardui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksysguard )
install( FILES ksysguardapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

