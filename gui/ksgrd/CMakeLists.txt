


########### next target ###############

set(ksgrd_LIB_SRCS
   SensorAgent.cpp
   SensorManager.cpp
   SensorShellAgent.cpp
   SensorSocketAgent.cpp
)


kde4_add_library(ksgrd SHARED ${ksgrd_LIB_SRCS})

target_link_libraries(ksgrd ${KDE4_KDECORE_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} )

set_target_properties(ksgrd PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS ksgrd EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install(FILES SensorAgent.h SensorClient.h SensorManager.h SensorShellAgent.h SensorSocketAgent.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksgrd COMPONENT Devel)



