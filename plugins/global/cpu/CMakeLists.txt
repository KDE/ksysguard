add_library(ksysguard_plugin_cpu MODULE  cpu.cpp cpuplugin.cpp linuxcpu.cpp)

target_link_libraries(ksysguard_plugin_cpu Qt5::Core KSysGuard::StatsBackend KF5::CoreAddons KF5::I18n)
install(TARGETS ksysguard_plugin_cpu DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
