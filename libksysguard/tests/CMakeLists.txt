if(KDE4_BUILD_TESTS)

  add_test(processtest ${EXECUTABLE_OUTPUT_PATH}/processtest)

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../processcore/ )

  # Tests don't need to go into toplevel/bin, they are fine in the current dir.
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

  kde4_automoc(processtest.cpp)

  kde4_add_executable(processtest RUN_UNINSTALLED processtest.cpp)

  target_link_libraries(processtest ksysguard ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

  ############################

  add_test(guitest ${EXECUTABLE_OUTPUT_PATH}/guitest)

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../processcore/  ${CMAKE_CURRENT_SOURCE_DIR}/../processui/ )

  # Tests don't need to go into toplevel/bin, they are fine in the current dir.
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

  kde4_automoc(guitest.cpp)

  kde4_add_executable(guitest RUN_UNINSTALLED guitest.cpp)

  target_link_libraries(guitest ksysguard processui ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})


  ############################################

endif(KDE4_BUILD_TESTS)

