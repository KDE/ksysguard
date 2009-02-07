
add_subdirectory( ksgrd ) 

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/ksgrd/
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ 
   ${CMAKE_SOURCE_DIR}/workspace
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard/processcore )


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
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ProcessController.cc 
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
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerDlgWidget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerSettingsWidget.ui
)

set(ksysguard_KDEINIT_SRCS ${libsensordisplays_SRCS}
   SensorBrowser.cc 
   WorkSheet.cc
   WorkSheetSettings.cc 
   Workspace.cc
   HostConnector.cc
   StyleEngine.cc
   ksysguard.cc )



kde4_add_kdeinit_executable(ksysguard ${ksysguard_KDEINIT_SRCS})

target_link_libraries(kdeinit_ksysguard processui ${KDE4_KIO_LIBS} ksgrd ${KDE4_PLASMA_LIBS} ${KDE4_KNEWSTUFF2_LIBS})

install(TARGETS kdeinit_ksysguard ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS ksysguard         ${INSTALL_TARGETS_DEFAULT_ARGS})
########### next target ###############

set(setscheduler_SRCS setscheduler.c)
add_executable(setscheduler ${setscheduler_SRCS})
install(TARGETS setscheduler ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES ksysguard.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES ProcessTable.sgrd SystemLoad2.sgrd DESTINATION  ${DATA_INSTALL_DIR}/ksysguard )
install( FILES ksysguardui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksysguard )
install( FILES ksysguard.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

