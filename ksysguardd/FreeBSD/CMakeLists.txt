INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

ADD_LIBRARY(libksysguardd STATIC
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
