include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set(ksysguardwidgets_PART_SRCS
   ksysguardprocesslist.cpp
   ProcessFilter.cc
   ProcessModel.cc
   ReniceDlg.cc
)

kde4_add_ui_files( ksysguardwidgets_PART_SRCS
   ReniceDlgUi.ui
   ProcessWidgetUI.ui
)

kde4_automoc(${ksysguardwidgets_PART_SRCS})

KDE4_ADD_WIDGET_FILES(${ksysguardwidgets_PART_SRCS} ksysguard.widgets)

kde4_add_plugin(ksysguardwidgets ${ksysguardwidgets_PART_SRCS})

target_link_libraries(ksysguardwidgets ${KDE4_KIO_LIBS} ksysguard)

install(TARGETS ksysguardwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )
