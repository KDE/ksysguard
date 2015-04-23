project(ksysguard)
set(PROJECT_VERSION "5.3.0")
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.4.0")

set(KSYSGUARD_VERSION 4.98.0)
set(KSYSGUARD_STRING_VERSION "${KSYSGUARD_VERSION}")

find_package(ECM 1.5.0 REQUIRED NO_MODULE)
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
find_package(KF5 REQUIRED COMPONENTS
    CoreAddons
    I18n
    ItemViews
    NewStuff
    Config
    IconThemes
    KDELibs4Support #for SensorDisplay.cpp
)

find_package(KF5 REQUIRED COMPONENTS SysGuard)

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

