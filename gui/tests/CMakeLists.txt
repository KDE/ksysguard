# set( ksysguarddtest_SRCS ksysguarddtest.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorAgent.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorManager.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorSocketAgent.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorShellAgent.cpp)
#
# ecm_add_test(${ksysguarddtest_SRCS}
#     TEST_NAME "ksysguard-ksysguarddtest"
#     LINK_LIBRARIES
#     Qt5::Test
#     Qt5::Network
#     Qt5::Widgets
#     KF5::ConfigCore
#     KF5::CoreAddons
#     KF5::I18n
# )
