


########### install files ###############

install_files( ${SYSCONF_INSTALL_DIR} FILES ksysguarddrc )
install_files( ${DATA_INSTALL_DIR}/ksysguard FILES eventsrc )




#original Makefile.am contents follow:

#ksysguard_example_datadir = $(kde_datadir)/ksysguard
#ksysguard_example_data_DATA = eventsrc
#
#etcdir = $(sysconfdir)
#etc_DATA = ksysguarddrc
