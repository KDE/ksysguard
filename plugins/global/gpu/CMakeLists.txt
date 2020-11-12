set(KSYSGUARD_GPU_PLUGIN_SOURCES
    GpuPlugin.cpp
    GpuBackend.cpp
    GpuDevice.cpp
    LinuxAmdGpu.cpp
    LinuxNvidiaGpu.cpp
    LinuxBackend.cpp
    NvidiaSmiProcess.cpp
    AllGpus.cpp
)

add_library(ksysguard_plugin_gpu MODULE ${KSYSGUARD_GPU_PLUGIN_SOURCES})
target_link_libraries(ksysguard_plugin_gpu Qt5::Core KSysGuard::StatsBackend KF5::CoreAddons KF5::I18n UDev::UDev)

install(TARGETS ksysguard_plugin_gpu DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard)
