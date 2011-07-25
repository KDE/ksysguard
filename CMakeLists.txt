if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   add_subdirectory( gui )
   add_subdirectory( pics )
   add_subdirectory( example )

   ########### install files ###############

   kde4_install_icons( ${ICON_INSTALL_DIR}  )
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

add_subdirectory( ksysguardd )

