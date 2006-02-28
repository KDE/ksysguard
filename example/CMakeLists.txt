kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files(  FILES ksysguarddrc )
install_files( ${DATA_INSTALL_DIR}/ksysguard FILES eventsrc )

kde4_footer()



#original Makefile.am contents follow:

#ksysguard_example_datadir = $(kde_datadir)/ksysguard
#ksysguard_example_data_DATA = eventsrc
#
#etcdir = $(sysconfdir)
#etc_DATA = ksysguarddrc
