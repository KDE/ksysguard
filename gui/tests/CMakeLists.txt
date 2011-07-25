set( ksysguarddtest_SRCS ksysguarddtest.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorAgent.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorManager.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorSocketAgent.cpp ${CMAKE_SOURCE_DIR}/libs/ksysguard/ksgrd/SensorShellAgent.cpp)

kde4_add_unit_test( ksysguarddtest
    TESTNAME ksubmodule-ksysguarddtest
    ${ksysguarddtest_SRCS}
)

target_link_libraries( ksysguarddtest
    ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
)
