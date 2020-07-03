set(KSYSGUARD_NETWORK_PLUGIN_SOURCES
    NetworkPlugin.cpp
    NetworkDevice.cpp
    NetworkBackend.cpp
    NetworkManagerBackend.cpp
)

add_library(ksysguard_globalplugin_network MODULE ${KSYSGUARD_NETWORK_PLUGIN_SOURCES})
target_link_libraries(ksysguard_globalplugin_network Qt5::Core Qt5::Gui Qt5::DBus KSysGuard::StatsBackend KF5::CoreAddons KF5::I18n KF5::NetworkManagerQt)

install(TARGETS ksysguard_globalplugin_network DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
