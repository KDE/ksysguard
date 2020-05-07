add_library(ksysguard_plugin_nvidia MODULE nvidia.cpp)
target_link_libraries(ksysguard_plugin_nvidia Qt5::Core KSysGuard::ProcessCore KF5::I18n KF5::CoreAddons)
install(TARGETS ksysguard_plugin_nvidia DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard/process)
