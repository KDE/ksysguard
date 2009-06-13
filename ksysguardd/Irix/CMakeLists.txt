INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

KDE4_ADD_LIBRARY(libksysguardd STATIC 
            cpu.c
            LoadAvg.c
            Memory.c
            NetDev.c
            ProcessList.c)

