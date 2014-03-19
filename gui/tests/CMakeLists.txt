set( ksysguarddtest_SRCS ksysguarddtest.cpp ${CMAKE_SOURCE_DIR}/libksysguard/ksgrd/SensorAgent.cpp ${CMAKE_SOURCE_DIR}/libksysguard/ksgrd/SensorManager.cpp ${CMAKE_SOURCE_DIR}/libksysguard/ksgrd/SensorSocketAgent.cpp ${CMAKE_SOURCE_DIR}/libksysguard/ksgrd/SensorShellAgent.cpp)

kde4_add_unit_test( ksysguarddtest
    TESTNAME ksysguard-ksysguarddtest
    ${ksysguarddtest_SRCS}
)

target_link_libraries( ksysguarddtest
    Qt5::Test
    Qt5::Network
    Qt5::Widgets
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
)
