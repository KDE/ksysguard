cmake_minimum_required(VERSION 3.0)

project(ksysguard)
set(PROJECT_VERSION "5.21.80")

set(KSYSGUARD_VERSION 4.98.0)
set(KSYSGUARD_STRING_VERSION "${KSYSGUARD_VERSION}")

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.78")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckIncludeFiles)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(ECMAddTests)
include(ECMInstallIcons)
include(FeatureSummary)
include (ECMConfiguredInstall)
include (ECMGenerateDBusServiceFile)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    Test
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
    Solid
    WindowSystem
)

find_package(KSysGuard REQUIRED)

find_package(KF5NetworkManagerQt ${KF5_MIN_VERSION})
set_package_properties(KF5NetworkManagerQt PROPERTIES
    TYPE RECOMMENDED
    PURPOSE "Provides an improved backend for Network statistics"
)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
#add_definitions(-DQT_NO_CAST_FROM_ASCII)
#add_definitions(-DQT_NO_CAST_TO_ASCII)
#add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
   add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054200)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(Sensors)
set_package_properties(Sensors PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Allows to show sensor information")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(NL)
    set_package_properties(NL PROPERTIES
        TYPE REQUIRED
        PURPOSE "Used for gathering socket info via the sock_diag netlink subsystem and for the network plugin when NetworkManagerQt is not available."
        URL "https://github.com/thom311/libnl/"
    )
    find_package(libpcap)
    set_package_properties(
        libpcap PROPERTIES
        TYPE RECOMMENDED
        PURPOSE "libpcap is used for per-application network usage."
    )
endif()

if(libpcap_FOUND )
    set(BUILD_NETWORK_PLUGIN TRUE)
endif()

if (libpcap_FOUND)
    find_package(Libcap)
    set_package_properties(Libcap PROPERTIES
        TYPE OPTIONAL
        PURPOSE "Needed for setting capabilities of the per-application network plugin."
    )
endif()

find_package(UDev)
set_package_properties(
    UDev PROPERTIES
    TYPE RECOMMENDED
    PURPOSE "UDev is used for finding graphics cards."
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h)

add_subdirectory( gui )

add_subdirectory( doc )
add_subdirectory( pics )
add_subdirectory( example )

add_subdirectory( ksysguardd )

add_subdirectory(ksystemstats)
add_subdirectory(libkstats)

add_subdirectory( plugins )

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

