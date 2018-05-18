cmake_minimum_required(VERSION 3.0)

project(ksysguard)
set(PROJECT_VERSION "5.14.80")

set(KSYSGUARD_VERSION 4.98.0)
set(KSYSGUARD_STRING_VERSION "${KSYSGUARD_VERSION}")

set(QT_MIN_VERSION "5.9.0")
set(KF5_MIN_VERSION "5.42.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckIncludeFiles)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMInstallIcons)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    CoreAddons
    DBusAddons
    DocTools
    I18n
    IconThemes
    Init
    ItemViews
    KIO
    NewStuff
    Notifications
    WindowSystem
)

find_package(KF5 REQUIRED COMPONENTS SysGuard)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
#add_definitions(-DQT_NO_CAST_FROM_ASCII)
#add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)
#add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)


find_package(Sensors)
set_package_properties(Sensors PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Allows to show sensor information")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h)

add_subdirectory( gui )

add_subdirectory( doc )
add_subdirectory( pics )
add_subdirectory( example )

add_subdirectory( ksysguardd )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

