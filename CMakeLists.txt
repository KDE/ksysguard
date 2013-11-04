if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   add_subdirectory( gui )
   add_subdirectory( pics )
   add_subdirectory( example )
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

add_subdirectory( ksysguardd )

