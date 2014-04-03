project(ksysguard)
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

set(KSYSGUARD_VERSION 4.95.0)
set(KSYSGUARD_STRING_VERSION "${KSYSGUARD_VERSION}")

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckIncludeFiles)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMAddTests)
include(ECMInstallIcons)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)
find_package(KF5 CONFIG REQUIRED COMPONENTS
    CoreAddons
    I18n
    ItemViews
    NewStuff
    Config
    IconThemes
    KDE4Support #for SensorDisplay.cpp
)

find_package(KF5 CONFIG REQUIRED COMPONENTS SysGuard)

find_package(Sensors)
set_package_properties(Sensors PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Allows to show sensor information")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h)

add_subdirectory( gui )
add_subdirectory( doc )
add_subdirectory( pics )
add_subdirectory( example )

add_subdirectory( ksysguardd )
