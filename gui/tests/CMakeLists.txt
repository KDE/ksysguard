set( signalplottertest_SRCS signalplottertest.cpp ../SensorDisplayLib/SignalPlotter.cc)

kde4_add_unit_test( signalplottertest
    TESTNAME ksubmodule-signalplottertest
    ${signalplottertest_SRCS}
)

target_link_libraries( signalplottertest
    ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY}
)
