INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

ADD_LIBRARY(libksysguardd STATIC 
            LoadAvg.c
            Memory.c
            NetDev.c
            ProcessList.c)

if(EXISTS /dev/kstat)
  ADD_DEFINITIONS(-DHAVE_KSTAT)
  FIND_LIBRARY(KSTAT_LIBRARY NAMES kstat)
  TARGET_LINK_LIBRARIES(libksysguardd ${KSTAT_LIBRARY})
ENDIF(EXISTS /dev/kstat)
