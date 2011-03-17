cmake_minimum_required(VERSION 2.6.2)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/CContLib/ )


# Laurent: Verify that we install it into (kdeprefix)/etc/ and not into /etc
# otherwise it's necessary to change it.
# into kde 3.x it was installed into /etc/
add_definitions( -DKSYSGUARDDRCFILE=\"${SYSCONF_INSTALL_DIR}/ksysguarddrc\" )

add_definitions(-DOSTYPE_${CMAKE_SYSTEM_NAME})

macro_optional_find_package(Sensors)
macro_bool_to_01(SENSORS_FOUND HAVE_LMSENSORS)

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)


configure_file(config-ksysguardd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguardd.h)

if( ${CMAKE_SYSTEM_NAME} MATCHES "kFreeBSD" )
    add_subdirectory( "FreeBSD" )
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/FreeBSD )
else( ${CMAKE_SYSTEM_NAME} MATCHES "kFreeBSD" )
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
endif( ${CMAKE_SYSTEM_NAME} MATCHES "kFreeBSD" )

########### next target ###############

    set(libccont_SRCS 
        ${CMAKE_CURRENT_SOURCE_DIR}/CContLib/ccont.c )

    set(ksysguardd_SRCS ${libccont_SRCS}
        Command.c 
        conf.c 
        ksysguardd.c 
        PWUIDCache.c )

    add_executable(ksysguardd ${ksysguardd_SRCS})

    target_link_libraries(ksysguardd libksysguardd)

   if( NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR NOT ${CMAKE_SYSTEM_NAME} MATCHES "kFreeBSD" )
      target_link_libraries(ksysguardd ${KDE4_KDEFAKES_LIBS})
   endif( NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR NOT ${CMAKE_SYSTEM_NAME} MATCHES "kFreeBSD" )

if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  target_link_libraries(ksysguardd kvm)
endif( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
if( ${CMAKE_SYSTEM_NAME} MATCHES "DragonFly" )
  message(STATUS "Adding kinfo library on DragonFly BSD")
  target_link_libraries(ksysguardd kinfo)
endif( ${CMAKE_SYSTEM_NAME} MATCHES "DragonFly" )
if( ${CMAKE_SYSTEM_NAME} MATCHES "SunOS" )
  target_link_libraries(ksysguardd socket nsl)
endif( ${CMAKE_SYSTEM_NAME} MATCHES "SunOS" )

install(TARGETS ksysguardd ${INSTALL_TARGETS_DEFAULT_ARGS})
