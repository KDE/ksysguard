set(SOURCES
    main.cpp
)

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/../ksysguard_iface.xml"
    PROPERTIES INCLUDE "../../libkstats/types.h" )
qt5_add_dbus_interface(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../ksysguard_iface.xml" kstatsiface)

add_executable(kstatsviewer ${SOURCES})
target_link_libraries(kstatsviewer Qt::Core Qt::DBus KSysGuard::Formatter)

install(TARGETS kstatsviewer DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
