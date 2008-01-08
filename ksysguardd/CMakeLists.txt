include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib/ )


# Laurent: Verify that we install it into (kdeprefix)/etc/ and not into /etc
# otherwise it's necessary to change it.
# into kde 3.x it was installed into /etc/
ADD_DEFINITIONS( -DKSYSGUARDDRCFILE=\"\\"${SYSCONF_INSTALL_DIR}/ksysguarddrc\\"\" )

ADD_DEFINITIONS(-DOSTYPE_${CMAKE_SYSTEM_NAME})

macro_optional_find_package(Sensors)
macro_bool_to_01(SENSORS_FOUND HAVE_LMSENSORS)
FIND_LIBRARY(XRES_LIBRARY XRes)
FIND_LIBRARY(X11_LIBRARY  X11)
FIND_LIBRARY(XEXT_LIBRARY Xext)

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)


if(XRES_LIBRARY)
  set(HAVE_XRES 1)
endif(XRES_LIBRARY)
configure_file(config-ksysguardd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguardd.h)

if( ${CMAKE_SYSTEM_NAME} MATCHES "DragonFly" )
  add_subdirectory( "FreeBSD" )
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/FreeBSD )
else( ${CMAKE_SYSTEM_NAME} MATCHES "DragonFly" )
  if( ${CMAKE_SYSTEM_NAME} MATCHES "SunOS" )
    add_subdirectory( "Solaris" )
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Solaris )
  else( ${CMAKE_SYSTEM_NAME} MATCHES "SunOS" )
    add_subdirectory( ${CMAKE_SYSTEM_NAME} )
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME} )
  endif( ${CMAKE_SYSTEM_NAME} MATCHES "SunOS" )
endif( ${CMAKE_SYSTEM_NAME} MATCHES "DragonFly" )

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
TARGET_LINK_LIBRARIES(ksysguardd libksysguardd ${KDE4_KDEFAKES_LIBS} ${X11_LIBRARY} ${XEXT_LIBRARY})
if(XRES_LIBRARY)
  TARGET_LINK_LIBRARIES(ksysguardd ${XRES_LIBRARY})
endif(XRES_LIBRARY)
if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  TARGET_LINK_LIBRARIES(ksysguardd kvm)
endif( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
if( ${CMAKE_SYSTEM_NAME} MATCHES "DragonFly" )
  message(STATUS "Adding kinfo library on DragonFlyBSD")
  TARGET_LINK_LIBRARIES(ksysguardd kinfo)
endif( ${CMAKE_SYSTEM_NAME} MATCHES "DragonFly" )
if( ${CMAKE_SYSTEM_NAME} MATCHES "SunOS" )
  TARGET_LINK_LIBRARIES(ksysguardd socket nsl)
endif( ${CMAKE_SYSTEM_NAME} MATCHES "SunOS" )

install(TARGETS ksysguardd DESTINATION ${BIN_INSTALL_DIR})

