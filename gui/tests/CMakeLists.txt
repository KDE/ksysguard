set( ksysguarddtest_SRCS ksysguarddtest.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorAgent.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorManager.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorSocketAgent.cpp ${libksysguard_SOURCE_DIR}/ksgrd/SensorShellAgent.cpp)

ecm_add_test(${ksysguarddtest_SRCS}
    TEST_NAME "ksysguard-ksysguarddtest"
    LINK_LIBRARIES
    Qt5::Test
    Qt5::Network
    Qt5::Widgets
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
)
