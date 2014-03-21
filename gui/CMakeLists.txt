include_directories(
   SensorDisplayLib/
)


########### next target ###############

set(libsensordisplays_SRCS
   	SensorDisplayLib/SensorDisplay.cpp
   	SensorDisplayLib/BarGraph.cpp
   	SensorDisplayLib/DancingBars.cpp
   	SensorDisplayLib/DancingBarsSettings.cpp
   	SensorDisplayLib/DummyDisplay.cpp
   	SensorDisplayLib/FancyPlotter.cpp
   	SensorDisplayLib/FancyPlotterSettings.cpp
   	SensorDisplayLib/ListView.cpp
   	SensorDisplayLib/LogFile.cpp
   	SensorDisplayLib/MultiMeter.cpp
   	SensorDisplayLib/MultiMeterSettings.cpp
   	SensorDisplayLib/ProcessController.cpp
   	SensorDisplayLib/SensorLogger.cpp
   	SensorDisplayLib/SensorLoggerDlg.cpp
   	SensorDisplayLib/SensorLoggerSettings.cpp
   	SensorDisplayLib/SensorModel.cpp
   	SensorDisplayLib/ListViewSettings.cpp
#   	SensorDisplayLib/modeltest.cpp
	)

qt5_wrap_ui( libsensordisplays_SRCS
   SensorDisplayLib/ListViewSettingsWidget.ui
   SensorDisplayLib/LogFileSettings.ui
   SensorDisplayLib/MultiMeterSettingsWidget.ui
   SensorDisplayLib/SensorLoggerDlgWidget.ui
   SensorDisplayLib/SensorLoggerSettingsWidget.ui
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



kf5_add_kdeinit_executable(ksysguard ${ksysguard_KDEINIT_SRCS})

target_link_libraries(kdeinit_ksysguard
    KF5::ProcessUi
    KF5::SignalPlotter
    KF5::ItemViews
    KF5::NewStuff
    KF5::SysGuard
    KF5::I18n
    KF5::IconThemes
    KF5::KDE4Support)

install(TARGETS kdeinit_ksysguard ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS ksysguard         ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( PROGRAMS ksysguard.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES ProcessTable.sgrd SystemLoad2.sgrd DESTINATION  ${DATA_INSTALL_DIR}/ksysguard )
install( FILES ksysguardui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksysguard )
install( FILES ksysguard.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

