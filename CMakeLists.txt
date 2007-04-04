
project(ksysguard)

add_definitions (-DQT3_SUPPORT)

add_subdirectory( CContLib )
add_subdirectory( gui )
add_subdirectory( pics )
add_subdirectory( example )
add_subdirectory( ksysguardd )
add_subdirectory( libksysguard )



########### install files ###############


kde4_install_icons( ${ICON_INSTALL_DIR}  )




