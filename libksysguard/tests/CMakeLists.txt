if(KDE4_BUILD_TESTS)
  add_test(processtest ${EXECUTABLE_OUTPUT_PATH}/processtest)
endif(KDE4_BUILD_TESTS)

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )

  # Tests don't need to go into toplevel/bin, they are fine in the current dir.
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

  kde4_automoc(processtest.cpp)

  kde4_add_test(processtest processtest.cpp)

  target_link_libraries(processtest processcore ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

  ############################
if(KDE4_BUILD_TESTS)
  add_test(guitest ${EXECUTABLE_OUTPUT_PATH}/guitest)
endif(KDE4_BUILD_TESTS)

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

  # Tests don't need to go into toplevel/bin, they are fine in the current dir.
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

  kde4_automoc(guitest.cpp)

  kde4_add_test(guitest guitest.cpp)

  target_link_libraries(guitest processui ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})


  ############################################


