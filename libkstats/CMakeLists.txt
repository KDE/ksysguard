set(ksgrdbackend_LIB_SRCS
    AggregateSensor.cpp
    SensorObject.cpp
    SensorContainer.cpp
    SensorPlugin.cpp
    SensorProperty.cpp
    SysFsSensor.cpp

)

add_library(ksgrdbackend ${ksgrdbackend_LIB_SRCS})
add_library(KSysGuard::StatsBackend ALIAS ksgrdbackend)

target_link_libraries(ksgrdbackend PUBLIC Qt::Core Qt::DBus KSysGuard::Formatter)

set_target_properties(ksgrdbackend PROPERTIES
    EXPORT_NAME SysGuardBackend
)

install(TARGETS ksgrdbackend EXPORT libKSysGuardbackendTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# Headers are currently not installed as we don't offer compatability yet
