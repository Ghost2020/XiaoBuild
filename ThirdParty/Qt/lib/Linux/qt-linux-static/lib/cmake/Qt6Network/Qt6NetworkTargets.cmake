# Generated by CMake

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.8)
   message(FATAL_ERROR "CMake >= 3.0.0 required")
endif()
if(CMAKE_VERSION VERSION_LESS "3.0.0")
   message(FATAL_ERROR "CMake >= 3.0.0 required")
endif()
cmake_policy(PUSH)
cmake_policy(VERSION 3.0.0...3.31)
#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Protect against multiple inclusion, which would fail when already imported targets are added once more.
set(_cmake_targets_defined "")
set(_cmake_targets_not_defined "")
set(_cmake_expected_targets "")
foreach(_cmake_expected_target IN ITEMS Qt6::Network Qt6::NetworkPrivate)
  list(APPEND _cmake_expected_targets "${_cmake_expected_target}")
  if(TARGET "${_cmake_expected_target}")
    list(APPEND _cmake_targets_defined "${_cmake_expected_target}")
  else()
    list(APPEND _cmake_targets_not_defined "${_cmake_expected_target}")
  endif()
endforeach()
unset(_cmake_expected_target)
if(_cmake_targets_defined STREQUAL _cmake_expected_targets)
  unset(_cmake_targets_defined)
  unset(_cmake_targets_not_defined)
  unset(_cmake_expected_targets)
  unset(CMAKE_IMPORT_FILE_VERSION)
  cmake_policy(POP)
  return()
endif()
if(NOT _cmake_targets_defined STREQUAL "")
  string(REPLACE ";" ", " _cmake_targets_defined_text "${_cmake_targets_defined}")
  string(REPLACE ";" ", " _cmake_targets_not_defined_text "${_cmake_targets_not_defined}")
  message(FATAL_ERROR "Some (but not all) targets in this export set were already defined.\nTargets Defined: ${_cmake_targets_defined_text}\nTargets not yet defined: ${_cmake_targets_not_defined_text}\n")
endif()
unset(_cmake_targets_defined)
unset(_cmake_targets_not_defined)
unset(_cmake_expected_targets)


# Create imported target Qt6::Network
add_library(Qt6::Network STATIC IMPORTED)

set_target_properties(Qt6::Network PROPERTIES
  COMPATIBLE_INTERFACE_STRING "QT_MAJOR_VERSION"
  INTERFACE_COMPILE_DEFINITIONS "QT_NETWORK_LIB"
  INTERFACE_COMPILE_OPTIONS "-fPIC"
  INTERFACE_INCLUDE_DIRECTORIES "/media/ywcloud/DiskSSD/github/qtbase-6.8/build/Linux/include;/media/ywcloud/DiskSSD/github/qtbase-6.8/build/Linux/include/QtNetwork"
  INTERFACE_LINK_LIBRARIES "Qt6::Core;\$<LINK_ONLY:Qt6::CorePrivate>;\$<LINK_ONLY:Qt6::PlatformModuleInternal>;\$<LINK_ONLY:WrapZLIB::WrapZLIB>;\$<LINK_ONLY:WrapResolv::WrapResolv>;\$<LINK_ONLY:GSSAPI::GSSAPI>"
  INTERFACE_QT_MAJOR_VERSION "6"
  INTERFACE_SOURCES "\$<\$<BOOL:\$<TARGET_PROPERTY:QT_CONSUMES_METATYPES>>:/media/ywcloud/DiskSSD/github/qtbase-6.8/build/Linux/src/network/meta_types/qt6network_release_metatypes.json>"
  MODULE_PLUGIN_TYPES "networkaccess;networkinformation;tls"
  QT_DISABLED_PRIVATE_FEATURES "libproxy;res_setservers;networklistmanager"
  QT_DISABLED_PUBLIC_FEATURES "getifaddrs;ipv6ifname;securetransport;schannel;sctp;brotli;sspi"
  QT_ENABLED_PRIVATE_FEATURES "libresolv;linux_netlink;system_proxies;publicsuffix_qt;publicsuffix_system"
  QT_ENABLED_PUBLIC_FEATURES "ssl;dtls;ocsp;http;udpsocket;networkproxy;socks5;networkinterface;networkdiskcache;localserver;dnslookup;gssapi;topleveldomain"
  QT_QMAKE_PRIVATE_CONFIG ""
  QT_QMAKE_PUBLIC_CONFIG ""
  QT_QMAKE_PUBLIC_QT_CONFIG ""
  _qt_config_module_name "network"
  _qt_is_internal_library "TRUE"
  _qt_is_internal_target "TRUE"
  _qt_is_public_module "TRUE"
  _qt_module_has_headers "ON"
  _qt_module_has_private_headers "TRUE"
  _qt_module_has_public_headers "TRUE"
  _qt_module_include_name "QtNetwork"
  _qt_module_interface_name "Network"
  _qt_package_name "Qt6Network"
  _qt_package_version "6.8.3"
  _qt_private_module_target_name "NetworkPrivate"
  _qt_sbom_spdx_id "SPDXRef-Package-qtbase-qt-module-Network"
  _qt_sbom_spdx_relative_installed_repo_document_path "sbom/qtbase-6.8.3.spdx"
  _qt_sbom_spdx_repo_document_namespace "https://qt.io/spdxdocs/qtbase-6.8.3"
  _qt_sbom_spdx_repo_project_name_lowercase "qtbase"
)

# Create imported target Qt6::NetworkPrivate
add_library(Qt6::NetworkPrivate INTERFACE IMPORTED)

set_target_properties(Qt6::NetworkPrivate PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "/media/ywcloud/DiskSSD/github/qtbase-6.8/build/Linux/src/network;\$<\$<BOOL:\$<TARGET_PROPERTY:Qt6::Network,_qt_module_has_private_headers>>:/media/ywcloud/DiskSSD/github/qtbase-6.8/build/Linux/include/QtNetwork/6.8.3>;\$<\$<BOOL:\$<TARGET_PROPERTY:Qt6::Network,_qt_module_has_private_headers>>:/media/ywcloud/DiskSSD/github/qtbase-6.8/build/Linux/include/QtNetwork/6.8.3/QtNetwork>;\$<\$<BOOL:\$<TARGET_PROPERTY:Qt6::Network,_qt_module_has_private_headers>>:>;\$<\$<BOOL:\$<TARGET_PROPERTY:Qt6::Network,_qt_module_has_private_headers>>:>"
  INTERFACE_LINK_LIBRARIES "Qt6::CorePrivate;Qt6::Network"
  _qt_config_module_name "network_private"
  _qt_is_private_module "TRUE"
  _qt_package_name "Qt6Network"
  _qt_package_version "6.8.3"
  _qt_public_module_target_name "Network"
  _qt_sbom_spdx_id "SPDXRef-Package-qtbase-qt-module-Network"
  _qt_sbom_spdx_relative_installed_repo_document_path "sbom/qtbase-6.8.3.spdx"
  _qt_sbom_spdx_repo_document_namespace "https://qt.io/spdxdocs/qtbase-6.8.3"
  _qt_sbom_spdx_repo_project_name_lowercase "qtbase"
)

# Import target "Qt6::Network" for configuration "Release"
set_property(TARGET Qt6::Network APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Qt6::Network PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LINK_INTERFACE_MULTIPLICITY_RELEASE "3"
  IMPORTED_LOCATION_RELEASE "/media/ywcloud/DiskSSD/github/qtbase-6.8/build/Linux/lib/libQt6Network.a"
  )

# Make sure the targets which have been exported in some other
# export set exist.
unset(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets)
foreach(_target "Qt6::Core" "Qt6::CorePrivate" "Qt6::PlatformModuleInternal" )
  if(NOT TARGET "${_target}" )
    set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets "${${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets} ${_target}")
  endif()
endforeach()

if(DEFINED ${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets)
  if(CMAKE_FIND_PACKAGE_NAME)
    set( ${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
    set( ${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE "The following imported targets are referenced, but are missing: ${${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets}")
  else()
    message(FATAL_ERROR "The following imported targets are referenced, but are missing: ${${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets}")
  endif()
endif()
unset(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets)

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
cmake_policy(POP)
