
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )


kde4_add_unit_test(processtest processtest.cpp)

target_link_libraries(processtest processcore ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

  ############################
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )


kde4_add_unit_test(guitest guitest.cpp)

target_link_libraries(guitest processui ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})


############################################


