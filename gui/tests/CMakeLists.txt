set( ksysguarddtest_SRCS ksysguarddtest.cpp ../ksgrd/SensorAgent.cc ../ksgrd/SensorManager.cc ../ksgrd/SensorSocketAgent.cc ../ksgrd/SensorShellAgent.cc)

kde4_add_unit_test( ksysguarddtest
    TESTNAME ksubmodule-ksysguarddtest
    ${ksysguarddtest_SRCS}
)

target_link_libraries( ksysguarddtest
    ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
)
