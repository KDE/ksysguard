add_library(ksysguard_plugin_lmsensors MODULE lmsensors.cpp SensorsFeatureSensor.cpp)

target_include_directories(ksysguard_plugin_lmsensors PRIVATE ${SENSORS_INCLUDE_DIR})
target_link_libraries(ksysguard_plugin_lmsensors PRIVATE Qt5::Core KSysGuard::StatsBackend KF5::CoreAddons KF5::I18n ${SENSORS_LIBRARIES})

install(TARGETS ksysguard_plugin_lmsensors DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
