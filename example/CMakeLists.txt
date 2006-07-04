


########### install files ###############

install( FILES ksysguarddrc  DESTINATION  ${SYSCONF_INSTALL_DIR} )
install( FILES eventsrc  DESTINATION  ${DATA_INSTALL_DIR}/ksysguard )




#original Makefile.am contents follow:

#ksysguard_example_datadir = $(kde_datadir)/ksysguard
#ksysguard_example_data_DATA = eventsrc
#
#etcdir = $(sysconfdir)
#etc_DATA = ksysguarddrc
