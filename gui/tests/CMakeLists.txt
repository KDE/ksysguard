set( ksysguarddtest_SRCS ksysguarddtest.cpp ../ksgrd/SensorAgent.cpp ../ksgrd/SensorManager.cpp ../ksgrd/SensorSocketAgent.cpp ../ksgrd/SensorShellAgent.cpp)

kde4_add_unit_test( ksysguarddtest
    TESTNAME ksubmodule-ksysguarddtest
    ${ksysguarddtest_SRCS}
)

target_link_libraries( ksysguarddtest
    ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
)
