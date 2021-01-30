include(ECMAddTests)

set(SOURCES
    main.cpp
)

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/../ksysguard_iface.xml"
    PROPERTIES INCLUDE "../../libkstats/types.h" )
qt5_add_dbus_interface(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../ksysguard_iface.xml" kstatsiface)

ecm_add_test(
    ${SOURCES}
    TEST_NAME ksystemstatstest
    LINK_LIBRARIES Qt::Test ksystemstats_core
)
