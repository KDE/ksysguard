set(KSYSGUARD_KSGRD_PLUGIN_SOURCES
    ksgrdiface.cpp
)

add_library(ksysguard_ksgrd MODULE ${KSYSGUARD_KSGRD_PLUGIN_SOURCES})
target_link_libraries(ksysguard_ksgrd Qt::Core Qt::DBus KSysGuard::StatsBackend KSysGuard::SysGuard KF5::CoreAddons KF5::I18n)

install(TARGETS ksysguard_ksgrd DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
