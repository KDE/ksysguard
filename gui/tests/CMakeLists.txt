set( ksysguarddtest_SRCS ksysguarddtest.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorAgent.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorManager.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorSocketAgent.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorShellAgent.cpp)

kde4_add_unit_test( ksysguarddtest
    TESTNAME ksysguard-ksysguarddtest
    ${ksysguarddtest_SRCS}
)

target_link_libraries( ksysguarddtest
    Qt5::Test
    Qt5::Network
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
)
