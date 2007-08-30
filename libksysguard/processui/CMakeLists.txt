
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )

set(processui_LIB_SRCS
   ksysguardprocesslist.cpp
   ProcessFilter.cc
   ProcessModel.cc
   ReniceDlg.cc
)

kde4_add_ui_files( processui_LIB_SRCS
   ReniceDlgUi.ui
   ProcessWidgetUI.ui
)


kde4_add_library(processui SHARED ${processui_LIB_SRCS})

target_link_libraries(processui ${KDE4_KIO_LIBS} processcore)

set_target_properties(processui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )


install(TARGETS processui DESTINATION ${LIB_INSTALL_DIR} )

#----------------------

set(ksysguardwidgets_PART_SRCS
   ${CMAKE_CURRENT_BINARY_DIR}/ksysguardwidgets.cpp
)

KDE4_ADD_WIDGET_FILES(${ksysguardwidgets_PART_SRCS} ksysguard.widgets)

kde4_add_plugin(ksysguardwidgets ${ksysguardwidgets_PART_SRCS})

target_link_libraries(ksysguardwidgets ${KDE4_KIO_PARTS} processui)

install(TARGETS ksysguardwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )

install( FILES ProcessModel.h ProcessFilter.h ksysguardprocesslist.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard)
