set(KSYSGUARD_NETWORK_PLUGIN_SOURCES
    NetworkPlugin.cpp
    NetworkDevice.cpp
    NetworkBackend.cpp
    AllDevicesObject.cpp
)

if (KF5NetworkManagerQt_FOUND)
    set(KSYSGUARD_NETWORK_PLUGIN_SOURCES ${KSYSGUARD_NETWORK_PLUGIN_SOURCES} NetworkManagerBackend.cpp)
    add_definitions(-DNETWORKMANAGER_FOUND)
endif()

add_library(ksysguard_globalplugin_network MODULE ${KSYSGUARD_NETWORK_PLUGIN_SOURCES})
target_link_libraries(ksysguard_globalplugin_network PRIVATE Qt5::Core Qt5::Gui Qt5::DBus KSysGuard::StatsBackend KF5::CoreAddons KF5::I18n)

if (KF5NetworkManagerQt_FOUND)
    target_link_libraries(ksysguard_globalplugin_network PRIVATE KF5::NetworkManagerQt)
endif()
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_sources(ksysguard_globalplugin_network PRIVATE RtNetlinkBackend.cpp)
    target_link_libraries(ksysguard_globalplugin_network PRIVATE ${NL_LIBRARIES})
    target_include_directories(ksysguard_globalplugin_network PRIVATE ${NL_INCLUDE_DIRS})
endif()

install(TARGETS ksysguard_globalplugin_network DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
