include_directories(
   SensorDisplayLib/
)


########### next target ###############

add_definitions(-DTRANSLATION_DOMAIN=\"ksysguard\")

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

ki18n_wrap_ui( libsensordisplays_SRCS
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
target_compile_definitions(kdeinit_ksysguard PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(kdeinit_ksysguard
    KSysGuard::ProcessUi
    KSysGuard::SysGuard
    KSysGuard::SignalPlotter
    KF5::DBusAddons
    KF5::ItemViews
    KF5::KIOWidgets
    KF5::NewStuff
    KF5::Notifications
    KF5::I18n
    KF5::IconThemes
    KF5::WindowSystem)

install(TARGETS kdeinit_ksysguard ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS ksysguard         ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( PROGRAMS org.kde.ksysguard.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES org.kde.ksysguard.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )
install( FILES ProcessTable.sgrd SystemLoad2.sgrd DESTINATION  ${KDE_INSTALL_DATADIR}/ksysguard )
install( FILES ksysguardui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/ksysguard )
install( FILES ksysguard.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})

