if(KDE4_BUILD_TESTS)

  add_test(processtest ${EXECUTABLE_OUTPUT_PATH}/processtest)

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

  # Tests don't need to go into toplevel/bin, they are fine in the current dir.
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

  kde4_automoc(processtest.cpp)

  kde4_add_executable(processtest RUN_UNINSTALLED processtest.cpp)

  target_link_libraries(processtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})


  ############################################

endif(KDE4_BUILD_TESTS)

