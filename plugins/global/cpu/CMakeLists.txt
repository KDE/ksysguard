add_library(ksysguard_plugin_cpu MODULE  cpu.cpp cpuplugin.cpp)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(ksysguard_plugin_cpu PRIVATE linuxcpu.cpp)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_sources(ksysguard_plugin_cpu PRIVATE freebsdcpu.cpp)
endif()

target_link_libraries(ksysguard_plugin_cpu Qt5::Core KSysGuard::StatsBackend KF5::CoreAddons KF5::I18n)

if(SENSORS_FOUND)
    target_include_directories(ksysguard_plugin_cpu PRIVATE ${SENSORS_INCLUDE_DIR})
    target_link_libraries(ksysguard_plugin_cpu ${SENSORS_LIBRARIES})
    target_compile_definitions(ksysguard_plugin_cpu PRIVATE HAVE_SENSORS)
endif()

install(TARGETS ksysguard_plugin_cpu DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
