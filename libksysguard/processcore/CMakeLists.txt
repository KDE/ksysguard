
########### next target ###############

set(ksysguard_LIB_SRCS
   processes.cpp
   process.cpp
   processes_local_p.cpp
   processes_remote_p.cpp
)



kde4_add_library(processcore SHARED ${ksysguard_LIB_SRCS})

target_link_libraries(processcore ${KDE4_KDECORE_LIBS} )

set_target_properties(processcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS processcore DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install( FILES processes.h process.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard)

