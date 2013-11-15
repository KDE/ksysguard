include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

add_library(libksysguardd STATIC
            acpi.c
            apm.c
            cpuinfo.c
            diskstat.c
            loadavg.c
            logfile.c
            Memory.c
            netdev.c
            ProcessList.c
            stat.c
            uptime.c)
