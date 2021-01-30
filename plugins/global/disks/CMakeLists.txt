
add_library(ksysguard_plugin_disk MODULE  disks.cpp)
target_link_libraries(ksysguard_plugin_disk Qt::Core KSysGuard::StatsBackend KF5::CoreAddons KF5::I18n KF5::KIOCore KF5::Solid)

if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_link_libraries(ksysguard_plugin_disk geom devstat)
endif()

install(TARGETS ksysguard_plugin_disk DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
