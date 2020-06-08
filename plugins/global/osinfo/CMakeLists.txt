set(KSYSGUARD_OSINFO_PLUGIN_SOURCES
    osinfo.cpp
)

add_library(ksysguard_plugin_osinfo MODULE ${KSYSGUARD_OSINFO_PLUGIN_SOURCES})
target_link_libraries(ksysguard_plugin_osinfo Qt5::Core Qt5::Gui Qt5::DBus KSysGuard::StatsBackend KF5::CoreAddons KF5::I18n)

install(TARGETS ksysguard_plugin_osinfo DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
