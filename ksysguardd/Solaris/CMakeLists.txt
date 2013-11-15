include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

add_library(libksysguardd STATIC 
            LoadAvg.c
            Memory.c
            NetDev.c
            ProcessList.c)

if(EXISTS /dev/kstat)
  add_definitions(-DHAVE_KSTAT)
  find_library(KSTAT_LIBRARY NAMES kstat)
  target_link_libraries(libksysguardd ${KSTAT_LIBRARY})
endif()
