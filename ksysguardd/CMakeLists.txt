include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/CContLib/ )

# Laurent: Verify that we install it into (kdeprefix)/etc/ and not into /etc
# otherwise it's necessary to change it.
# into kde 3.x it was installed into /etc/
add_definitions( -DKSYSGUARDDRCFILE="${KDE_INSTALL_FULL_SYSCONFDIR}/ksysguarddrc" )

add_definitions(-DOSTYPE_${CMAKE_SYSTEM_NAME})

set(HAVE_LMSENSORS ${SENSORS_FOUND})

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
set(HAVE_SYS_INOTIFY_H ${SYS_INOTIFY_H_FOUND})


configure_file(config-ksysguardd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguardd.h)

if( ${CMAKE_SYSTEM_NAME} MATCHES "kFreeBSD" )
    add_subdirectory( "FreeBSD" )
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/FreeBSD )
else()
if( ${CMAKE_SYSTEM_NAME} MATCHES "DragonFly" )
    add_subdirectory( "FreeBSD" )
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/FreeBSD )
else()
    if( ${CMAKE_SYSTEM_NAME} MATCHES "SunOS" )
        add_subdirectory( "Solaris" )
        include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Solaris )
    else()
        add_subdirectory( ${CMAKE_SYSTEM_NAME} )
        include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME} )
    endif()
endif()
endif()

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

if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  target_link_libraries(ksysguardd kvm)
endif()
if( ${CMAKE_SYSTEM_NAME} MATCHES "DragonFly" )
  message(STATUS "Adding kinfo library on DragonFly BSD")
  target_link_libraries(ksysguardd kinfo)
endif()
if( ${CMAKE_SYSTEM_NAME} MATCHES "SunOS" )
  target_link_libraries(ksysguardd socket nsl)
endif()

install(TARGETS ksysguardd ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
