


########### next target ###############

set(ksgrd_LIB_SRCS 
   HostConnector.cc 
   SensorAgent.cc 
   SensorManager.cc 
   SensorShellAgent.cc 
   SensorSocketAgent.cc 
   StyleEngine.cc 
   StyleSettings.cc 
   TimerSettings.cc )

kde4_automoc(${ksgrd_LIB_SRCS})

kde4_add_library(ksgrd SHARED ${ksgrd_LIB_SRCS})

target_link_libraries(ksgrd  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

set_target_properties(ksgrd PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install(TARGETS ksgrd  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install(FILES HostConnector.h SensorAgent.h SensorClient.h SensorManager.h SensorShellAgent.h SensorSocketAgent.h StyleEngine.h StyleSettings.h TimerSettings.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksgrd )



