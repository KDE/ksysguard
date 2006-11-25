include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib/ )

ADD_DEFINITIONS( -DKSYSGUARDDRCFILE=\"\\"${CMAKE_INSTALL_PREFIX}${SYSCONF_INSTALL_DIR}/ksysguarddrc\\"\" )

ADD_DEFINITIONS(-DOSTYPE_${CMAKE_SYSTEM_NAME})

macro_optional_find_package(Sensors)
macro_bool_to_01(SENSORS_FOUND, HAVE_LMSENSORS)
FIND_LIBRARY(XRES_LIBRARY XRes)
FIND_LIBRARY(X11_LIBRARY  X11)
FIND_LIBRARY(XEXT_LIBRARY Xext)
if(XRES_LIBRARY)
  set(HAVE_XRES 1)
endif(XRES_LIBRARY)
configure_file(config-ksysguardd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguardd.h)

add_subdirectory( ${CMAKE_SYSTEM_NAME} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME} )


########### next target ###############

set(libccont_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/../CContLib/ccont.c )

set(ksysguardd_SRCS ${libccont_SRCS}
   Command.c 
   conf.c 
   ksysguardd.c 
   PWUIDCache.c )

add_executable(ksysguardd ${ksysguardd_SRCS})

ADD_DEPENDENCIES(ksysguardd libksysguardd)
TARGET_LINK_LIBRARIES(ksysguardd libksysguardd kdefakes ${X11_LIBRARY} ${XEXT_LIBRARY})
if(XRES_LIBRARY)
  TARGET_LINK_LIBRARIES(ksysguardd ${XRES_LIBRARY})
endif(XRES_LIBRARY)
if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  TARGET_LINK_LIBRARIES(ksysguardd kvm)
endif( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )

install(TARGETS ksysguardd DESTINATION ${BIN_INSTALL_DIR})

