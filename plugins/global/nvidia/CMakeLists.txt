set(KSYSGUARD_NVIDIA_PLUGIN_SOURCES
    nvidia.cpp
)

add_library(ksysguard_plugin_nvidiaglobal MODULE ${KSYSGUARD_NVIDIA_PLUGIN_SOURCES})
target_link_libraries(ksysguard_plugin_nvidiaglobal Qt5::Core Qt5::DBus PW5::SysGuardBackend KF5::CoreAddons KF5::I18n)

install(TARGETS ksysguard_plugin_nvidiaglobal DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
