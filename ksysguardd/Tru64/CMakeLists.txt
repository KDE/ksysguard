INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

KDE4_ADD_LIBRARY(libksysguardd STATIC 
            LoadAvg.c
            Memory.c
            NetDev.c)

