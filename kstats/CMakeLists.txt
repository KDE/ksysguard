set(SOURCES
    client.cpp
    ksysguarddaemon.cpp
)

set_source_files_properties("ksysguard_iface.xml"
    PROPERTIES INCLUDE "../ksysguard-backend/types.h" )
qt5_add_dbus_adaptor(SOURCES "ksysguard_iface.xml" ksysguarddaemon.h KSysGuardDaemon)

add_library(kstats_core STATIC ${SOURCES})
target_link_libraries(kstats_core PUBLIC Qt5::Core Qt5::DBus KF5::CoreAddons KSysGuard::StatsBackend )

add_executable(kstats main.cpp)
target_link_libraries(kstats kstats_core)

install(TARGETS kstats DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

kdbusaddons_generate_dbus_service_file(kstats org.kde.kstats ${KDE_INSTALL_FULL_BINDIR})

add_subdirectory(test)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
