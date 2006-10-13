


########### next target ###############

set(ksgrd_LIB_SRCS 
   SensorAgent.cc 
   SensorManager.cc 
   SensorShellAgent.cc 
   SensorSocketAgent.cc 
)

kde4_automoc(ksgrd ${ksgrd_LIB_SRCS})

kde4_add_library(ksgrd SHARED ${ksgrd_LIB_SRCS})

target_link_libraries(ksgrd  ${KDE4_KDECORE_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} )

set_target_properties(ksgrd PROPERTIES VERSION 4.0.0 SOVERSION 4 )
install(TARGETS ksgrd  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install(FILES SensorAgent.h SensorClient.h SensorManager.h SensorShellAgent.h SensorSocketAgent.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksgrd )



