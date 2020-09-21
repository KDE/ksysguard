add_library(ksysguard_plugin_power MODULE power.cpp)
target_link_libraries(ksysguard_plugin_power KF5::CoreAddons KF5::I18n KF5::Solid KSysGuard::StatsBackend)
install(TARGETS ksysguard_plugin_power DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)

