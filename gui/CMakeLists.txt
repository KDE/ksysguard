
add_subdirectory( tests )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard/signalplotter
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/ksysguard/processcore )


########### next target ###############

set(libsensordisplays_SRCS
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorDisplay.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/BarGraph.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/DancingBars.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/DancingBarsSettings.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/DummyDisplay.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/FancyPlotter.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/FancyPlotterSettings.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ListView.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/LogFile.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/MultiMeter.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/MultiMeterSettings.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ProcessController.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLogger.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerDlg.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorLoggerSettings.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/SensorModel.cpp
   	${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib/ListViewSettings.cpp
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
   ksortfilterproxymodel.cpp
   SensorBrowser.cpp
   WorkSheet.cpp
   WorkSheetSettings.cpp
   Workspace.cpp
   HostConnector.cpp
   StyleEngine.cpp
   ksysguard.cpp )



kde4_add_kdeinit_executable(ksysguard ${ksysguard_KDEINIT_SRCS})

target_link_libraries(kdeinit_ksysguard processui ksignalplotter ${KDE4_KIO_LIBS} ksgrd ${KDE4_KNEWSTUFF3_LIBS})

install(TARGETS kdeinit_ksysguard ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS ksysguard         ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( PROGRAMS ksysguard.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES ProcessTable.sgrd SystemLoad2.sgrd DESTINATION  ${DATA_INSTALL_DIR}/ksysguard )
install( FILES ksysguardui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksysguard )
install( FILES ksysguard.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

