if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    add_library(ksysguard_plugin_memory MODULE memory.cpp backend.cpp)

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_sources(ksysguard_plugin_memory PRIVATE linuxbackend.cpp)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        target_sources(ksysguard_plugin_memory PRIVATE freebsdbackend.cpp)
        target_link_libraries(ksysguard_plugin_memory kvm)
    endif()

    target_link_libraries(ksysguard_plugin_memory Qt::Core KSysGuard::StatsBackend KF5::CoreAddons KF5::I18n)

    install(TARGETS ksysguard_plugin_memory DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
endif()
