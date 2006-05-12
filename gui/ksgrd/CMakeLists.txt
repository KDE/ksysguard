


########### next target ###############

set(ksgrd_LIB_SRCS 
   HostConnector.cc 
   SensorAgent.cc 
   SensorManager.cc 
   SensorShellAgent.cc 
   SensorSocketAgent.cc 
   StyleEngine.cc 
   StyleSettings.cc 
   TimerSettings.cc )

kde4_automoc(${ksgrd_LIB_SRCS})

kde4_add_library(ksgrd SHARED ${ksgrd_LIB_SRCS})

target_link_libraries(ksgrd  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

set_target_properties(ksgrd PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} ksgrd )


########### install files ###############

install_files( /include/ksgrd FILES HostConnector.h  	SensorAgent.h  	SensorClient.h  	SensorManager.h  	SensorShellAgent.h  	SensorSocketAgent.h  	StyleEngine.h  	StyleSettings.h  	TimerSettings.h )




#original Makefile.am contents follow:

#
## set the include path for X, qt and KDE
#INCLUDES= -I$(srcdir)/../SensorDisplayLib $(all_includes)
# 
#lib_LTLIBRARIES = libksgrd.la
#
#libksgrd_la_LDFLAGS = -no-undefined -version-info 3:0:2 $(all_libraries)
#libksgrd_la_LIBADD = $(LIB_KDEUI)
#
## Which sources should be compiled for ksysguard.
#libksgrd_la_SOURCES = \
#	HostConnector.cc \
#	SensorAgent.cc \
#	SensorManager.cc \
#	SensorShellAgent.cc \
#	SensorSocketAgent.cc \
#	StyleEngine.cc \
#	StyleSettings.cc \
#	TimerSettings.cc
#
#ksgrdincludedir = $(includedir)/ksgrd
#ksgrdinclude_HEADERS = \
#	HostConnector.h \
#	SensorAgent.h \
#	SensorClient.h \
#	SensorManager.h \
#	SensorShellAgent.h \
#	SensorSocketAgent.h \
#	StyleEngine.h \
#	StyleSettings.h \
#	TimerSettings.h
#
## just to make sure, automake makes them 
#libksgrd_la_METASOURCES = AUTO
