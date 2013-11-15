include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set(LIBKSYSGUARDD_FILES
            acpi.c
            apm.c
            cpuinfo.c
            diskstat.c
            diskstats.c
            i8k.c
            loadavg.c
            logfile.c
            Memory.c
            netdev.c
            netstat.c
            ProcessList.c
            stat.c
            softraid.c
            uptime.c)

if(SENSORS_FOUND)
  set(LIBKSYSGUARDD_FILES ${LIBKSYSGUARDD_FILES} lmsensors.c)
endif()

if(EXISTS /proc/i8k)
  add_definitions(-DHAVE_I8K_SUPPORT)
endif()


add_library(libksysguardd STATIC 
            ${LIBKSYSGUARDD_FILES})
if(SENSORS_FOUND)
  target_link_libraries(libksysguardd ${SENSORS_LIBRARIES})
endif()

