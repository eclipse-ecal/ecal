# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2025 Continental Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================

cmake_minimum_required(VERSION 3.15)

include(CMakeDependentOption)

if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW) # Prevent HDF5 from overwriting our output paths
endif (POLICY CMP0077)

list(APPEND CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
  
set(eCAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

project(eCAL)
set(ECAL_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}")

# --------------------------------------------------------
# set msvc specific options
# --------------------------------------------------------
if(MSVC)
  message(STATUS "MSVC detected - Adding flags")
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /W4")
  # support Windows 7 and newer
  add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# --------------------------------------------------------
# set gcc specific options
# --------------------------------------------------------
if(UNIX)
  message(STATUS "GCC detected - Adding flags")
  add_definitions(-Wall -Wextra)

  set(ATOMIC_TEST_CODE "
      #include <atomic>
      int main() { std::atomic<int64_t> i(0); i++; return 0; }
  ")
  include(CheckCXXSourceCompiles)
  check_cxx_source_compiles("${ATOMIC_TEST_CODE}" ATOMIC_IS_BUILTIN)
  if (NOT ATOMIC_IS_BUILTIN)
      message(STATUS "Adding -latomic flag, as this plattform does not support 64bit atomic operations out of the box.")
      set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")
  endif ()
endif()

#---------------------------------------------------------
# Compatibility Options with eCAL 5 CMake Options
#---------------------------------------------------------

# Legacy Compatibility option for HAS_HDF5
if(DEFINED HAS_HDF5 AND NOT DEFINED ECAL_USE_HDF5)
  set(ECAL_USE_HDF5 ${HAS_HDF5})
  message(WARNING "The option HAS_HDF5 is deprecated and may be removed at any time. Please use ECAL_USE_HDF5 instead.")
endif()

# Legacy Compatibility option for HAS_QT
if(DEFINED HAS_QT AND NOT DEFINED ECAL_USE_QT)
  set(ECAL_USE_QT ${HAS_QT})
  message(WARNING "The option HAS_QT is deprecated and may be removed at any time. Please use ECAL_USE_QT instead.")
endif()

# Legacy Compatibility option for HAS_CURL
if(DEFINED HAS_CURL AND NOT DEFINED ECAL_USE_CURL)
  set(ECAL_USE_CURL ${HAS_CURL})
  message(WARNING "The option HAS_CURL is deprecated and may be removed at any time. Please use ECAL_USE_CURL instead.")
endif()

# Legacy Compatibility option for HAS_FTXUI
if(DEFINED HAS_FTXUI AND NOT DEFINED ECAL_USE_FTXUI)
  set(ECAL_USE_FTXUI ${HAS_FTXUI})
  message(WARNING "The option HAS_FTXUI is deprecated and may be removed at any time. Please use ECAL_USE_FTXUI instead.")
endif()

# Legacy Compatibility option for HAS_CAPNPROTO
if(DEFINED HAS_CAPNPROTO AND NOT DEFINED ECAL_USE_CAPNPROTO)
  set(ECAL_USE_CAPNPROTO ${HAS_CAPNPROTO})
  message(WARNING "The option HAS_CAPNPROTO is deprecated and may be removed at any time. Please use ECAL_USE_CAPNPROTO instead.")
endif()

# Legacy Compatibility option for HAS_FLATBUFFERS
if(DEFINED HAS_FLATBUFFERS AND NOT DEFINED ECAL_USE_FLATBUFFERS)
  set(ECAL_USE_FLATBUFFERS ${HAS_FLATBUFFERS})
  message(WARNING "The option HAS_FLATBUFFERS is deprecated and may be removed at any time. Please use ECAL_USE_FLATBUFFERS instead.")
endif()

# Legacy Compatibility option for HAS_MESSAGEPACK
if(DEFINED HAS_MESSAGEPACK AND NOT DEFINED ECAL_USE_MESSAGEPACK)
  set(ECAL_USE_MESSAGEPACK ${HAS_MESSAGEPACK})
  message(WARNING "The option HAS_MESSAGEPACK is deprecated and may be removed at any time. Please use ECAL_USE_MESSAGEPACK instead.")
endif()

# Legacy Compatibility option for BUILD_DOCS
if(DEFINED BUILD_DOCS AND NOT DEFINED ECAL_BUILD_DOCS)
  set(ECAL_BUILD_DOCS ${BUILD_DOCS})
  message(WARNING "The option BUILD_DOCS is deprecated and may be removed at any time. Please use ECAL_BUILD_DOCS instead.")
endif()

# Legacy Compatibility option for BUILD_APPS
if(DEFINED BUILD_APPS AND NOT DEFINED ECAL_BUILD_APPS)
  set(ECAL_BUILD_APPS ${BUILD_APPS})
  message(WARNING "The option BUILD_APPS is deprecated and may be removed at any time. Please use ECAL_BUILD_APPS instead.")
endif()

# Legacy Compatibility option for BUILD_SAMPLES
if(DEFINED BUILD_SAMPLES AND NOT DEFINED ECAL_BUILD_SAMPLES)
  set(ECAL_BUILD_SAMPLES ${BUILD_SAMPLES})
  message(WARNING "The option BUILD_SAMPLES is deprecated and may be removed at any time. Please use ECAL_BUILD_SAMPLES instead.")
endif()

# Legacy Compatibility option for BUILD_TIME
if(DEFINED BUILD_TIME AND NOT DEFINED ECAL_BUILD_TIMEPLUGINS)
  set(ECAL_BUILD_TIMEPLUGINS ${BUILD_TIME})
  message(WARNING "The option BUILD_TIME is deprecated and may be removed at any time. Please use ECAL_BUILD_TIMEPLUGINS instead.")
endif()

# Legacy Compatibility option for BUILD_C_BINDING
if(DEFINED BUILD_C_BINDING AND NOT DEFINED ECAL_BUILD_C_BINDING)
  set(ECAL_BUILD_C_BINDING ${BUILD_C_BINDING})
  message(WARNING "The option BUILD_C_BINDING is deprecated and may be removed at any time. Please use ECAL_BUILD_C_BINDING instead.")
endif()

# Legacy Compatibility option for BUILD_PY_BINDING
if(DEFINED BUILD_PY_BINDING AND NOT DEFINED ECAL_BUILD_PY_BINDING)
  set(ECAL_BUILD_PY_BINDING ${BUILD_PY_BINDING})
  message(WARNING "The option BUILD_PY_BINDING is deprecated and may be removed at any time. Please use ECAL_BUILD_PY_BINDING instead.")
endif()

# Legacy Compatibility option for BUILD_CSHARP_BINDING
if(DEFINED BUILD_CSHARP_BINDING AND NOT DEFINED ECAL_BUILD_CSHARP_BINDING)
  set(ECAL_BUILD_CSHARP_BINDING ${BUILD_CSHARP_BINDING})
  message(WARNING "The option BUILD_CSHARP_BINDING is deprecated and may be removed at any time. Please use ECAL_BUILD_CSHARP_BINDING instead.")
endif()

# Legacy Compatibility option for ECAL_NPCAP_SUPPORT
if(DEFINED ECAL_NPCAP_SUPPORT AND NOT DEFINED ECAL_USE_NPCAP)
  set(ECAL_USE_NPCAP ${ECAL_NPCAP_SUPPORT})
  message(WARNING "The option ECAL_NPCAP_SUPPORT is deprecated and may be removed at any time. Please use ECAL_USE_NPCAP instead.")
endif()

# Legacy Compatibility option for BUILD_ECAL_TESTS
if(DEFINED BUILD_ECAL_TESTS AND NOT DEFINED ECAL_BUILD_TESTS)
  set(ECAL_BUILD_TESTS ${BUILD_ECAL_TESTS})
  message(WARNING "The option BUILD_ECAL_TESTS is deprecated and may be removed at any time. Please use ECAL_BUILD_TESTS instead.")
endif()

# Legacy Compatibility option for CPACK_PACK_WITH_INNOSETUP
if(DEFINED CPACK_PACK_WITH_INNOSETUP AND NOT DEFINED ECAL_CPACK_PACK_WITH_INNOSETUP)
  set(ECAL_CPACK_PACK_WITH_INNOSETUP ${CPACK_PACK_WITH_INNOSETUP})
  message(WARNING "The option CPACK_PACK_WITH_INNOSETUP is deprecated and may be removed at any time. Please use ECAL_CPACK_PACK_WITH_INNOSETUP instead.")
endif()

# --------------------------------------------------------
# command line build options
# use it that way cmake .. -DECAL_BUILD_APPS=ON -DECAL_BUILD_SAMPLES=ON
# --------------------------------------------------------
option(ECAL_USE_HDF5                                "Platform supports HDF5 library"                                    ON)

option(ECAL_USE_QT                                  "Platform supports Qt 5 / 6 library."                               ON)

option(ECAL_USE_CURL                                "Build with CURL (i.e. upload support in the recorder app)"         ON)
option(ECAL_USE_FTXUI                               "Platform supports FTXUI library. Requires C++17 and up."           ON)

# Serialization format support
option(ECAL_USE_CAPNPROTO                           "Platform supports Cap'n Proto library"                            OFF)
option(ECAL_USE_FLATBUFFERS                         "Platform supports flatbuffers library"                            OFF)
option(ECAL_USE_MESSAGEPACK                         "Platform supports messagepack library"                            OFF)

option(ECAL_BUILD_DOCS                              "Build the eCAL documentation"                                     OFF)
option(ECAL_BUILD_APPS                              "Build the eCAL applications"                                       ON)
option(ECAL_BUILD_APP_SDK                           "Build SDK components of eCAL applications"         ${ECAL_BUILD_APPS})
option(ECAL_BUILD_SAMPLES                           "Build the eCAL samples"                                            ON)
option(ECAL_BUILD_DOCS_SAMPLES                      "Build the eCAL documentation samples"              ${ECAL_BUILD_DOCS})
option(ECAL_BUILD_TIMEPLUGINS                       "Build the eCAL time interfaces"                                    ON)
option(ECAL_BUILD_C_BINDING                         "Build eCAL C binding"                                              ON)
option(ECAL_BUILD_CSHARP_BINDING                    "Build eCAL C# binding"                                            OFF)
option(ECAL_BUILD_PY_BINDING                        "Build eCAL Python binding"                                        OFF)
option(ECAL_BUILD_TESTS                             "Build the eCAL google tests"                                      OFF)

option(ECAL_INSTALL_SAMPLE_SOURCES                  "Install the sources of eCAL samples"                               ON)

option(ECAL_USE_NPCAP                               "Enable the eCAL Npcap Receiver (i.e. the Win10 performance fix)"  OFF)
option(ECAL_USE_CLOCKLOCK_MUTEX                     "Use native mutex with monotonic clock (requires glibc >= 2.30)"   OFF)

# --------------------------------------------------------
# ecal core configuration
# --------------------------------------------------------
set(ECAL_CORE_USE_PROTOBUF                                                                                              ON)
set(ECAL_CORE_BUILD_SAMPLES                                                                          ${ECAL_BUILD_SAMPLES})
set(ECAL_CORE_BUILD_TESTS                                                                              ${ECAL_BUILD_TESTS})
set(ECAL_CORE_CONFIGURATION                                                                                             ON)
set(ECAL_CORE_COMMAND_LINE                                                                                              ON)
set(ECAL_CORE_REGISTRATION                                                                                              ON)
set(ECAL_CORE_MONITORING                                                                                                ON)
set(ECAL_CORE_PUBLISHER                                                                                                 ON)
set(ECAL_CORE_SUBSCRIBER                                                                                                ON)
set(ECAL_CORE_SERVICE                                                                                                   ON)
set(ECAL_CORE_TIMEGATE                                                                                                  ON)
set(ECAL_CORE_TIMEPLUGIN                                                                                                ON)
set(ECAL_CORE_NPCAP_SUPPORT                                                                              ${ECAL_USE_NPCAP})
set(ECAL_CORE_REGISTRATION_SHM                                                                                          ON)
set(ECAL_CORE_TRANSPORT_UDP                                                                                             ON)
set(ECAL_CORE_TRANSPORT_TCP                                                                                             ON)
set(ECAL_CORE_TRANSPORT_SHM                                                                                             ON)

# -----------------------
# eCAL Python configuration
# -----------------------
#set(ECAL_PYTHON_BUILD_SAMPLES                                                                        ${ECAL_BUILD_SAMPLES})
set(ECAL_PYTHON_BUILD_TESTS                                                                            ${ECAL_BUILD_TESTS})
set(ECAL_PYTHON_USE_HDF5                                                                                  ${ECAL_USE_HDF5})


# This should be ON, unless build ecal hdf5 for Matlab usage
option(ECAL_LINK_HDF5_SHARED                        "Link shared libs of HDF5"                                          ON)

option(ECAL_CPACK_PACK_WITH_INNOSETUP               "Create Innosetup installer for the Windows build"                  ON)

set(ECAL_INSTALL_PYTHON_DIR "bin" CACHE PATH "Location to install the Python extension modules. Might be set by setupdtools install.")

if(DEFINED SKBUILD_PROJECT_VERSION)
  message(STATUS
    "Using version from scikit-build-core: ${SKBUILD_PROJECT_VERSION}"
  )
  set(ECAL_BUILD_VERSION "${SKBUILD_PROJECT_VERSION}" CACHE STRING
    "Version provided by scikit-build-core and setuptools-scm" FORCE
  )
else()
  set(ECAL_BUILD_VERSION "0.0.0" CACHE STRING
    "Inject a build version if not building from a git repository"
  )
endif()




# there is a CMake issue with testing threading availibility via TEST_RUN
if(${CMAKE_CROSSCOMPILING})
  set(THREADS_PTHREAD_ARG "2" CACHE STRING "Forcibly set by CMakeLists.txt." FORCE)
endif()

include(build_location)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  message(STATUS "Compilation database will be created")
else()
  # to make it visible it is explicitly set to OFF
  set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
endif()

# Normalize backslashes from Windows paths
file(TO_CMAKE_PATH "${CMAKE_MODULE_PATH}" CMAKE_MODULE_PATH)
file(TO_CMAKE_PATH "${CMAKE_PREFIX_PATH}" CMAKE_PREFIX_PATH)
message(STATUS "Module Path: ${CMAKE_MODULE_PATH}")
message(STATUS "Prefix Path: ${CMAKE_PREFIX_PATH}")

# -------
# Select C++ standard
# Abseil requires everything to be built with a consistent C++ standard.
# If abseil is built with C++14 but a component building with C++17 tries to use
# abseil (eg: via protobuf) there will be a mismatch and possibly a build failure.
# This is most prevalent with string_view where C++14 will use the abseil type
# but C++17 use std::string_view
# -------

# C++14 is the baseline required standard
set(_ECAL_CXX_STD 14)

# Some dependencies require C++17, which we must detect and push to everything
if(ECAL_USE_QT)
  find_package(QT NAMES Qt6 Qt5 COMPONENTS Core QUIET)
  if(MSVC AND NOT "${QT_FOUND}")
    include("cmake/qt_msvc_path.cmake")
    autodetect_qt_msvc_dir()
  endif()
  if("${QT_VERSION_MAJOR}" VERSION_GREATER 5)
    # QT6 has C++17 usage requirements
    set(_ECAL_CXX_STD 17)
  endif()
endif()
if(ECAL_USE_FTXUI)
  # FTXUI requires C++17
  set(_ECAL_CXX_STD 17)
endif()

if("${CMAKE_SOURCE_DIR}" STREQUAL "${eCAL_SOURCE_DIR}" AND NOT DEFINED CMAKE_CXX_STANDARD)
  # eCAL is being compiled as the root project, and no standard version is set
  set(CMAKE_CXX_STANDARD "${_ECAL_CXX_STD}")
  message(STATUS "eCAL is using C++ ${_ECAL_CXX_STD}")
else()
  # eCAL is part of a larger build or the standard was externally set
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
if("${CMAKE_CXX_STANDARD}" VERSION_LESS "${_ECAL_CXX_STD}")
  message(FATAL_ERROR "eCAL requires CMAKE_CXX_STANDARD to be at least ${_ECAL_CXX_STD} (got: ${CMAKE_CXX_STANDARD})")
endif()

find_package(CMakeFunctions REQUIRED)

git_revision_information(DEFAULT ${ECAL_BUILD_VERSION})
set(eCAL_VERSION_MAJOR  ${GIT_REVISION_MAJOR})
set(eCAL_VERSION_MINOR  ${GIT_REVISION_MINOR})
set(eCAL_VERSION_PATCH  ${GIT_REVISION_PATCH})
set(eCAL_VERSION_STRING ${eCAL_VERSION_MAJOR}.${eCAL_VERSION_MINOR}.${eCAL_VERSION_PATCH})
set(eCAL_VERSION        ${eCAL_VERSION_STRING})
set(eCAL_COMPLETE_VERSION ${GIT_DESCRIBE_TAG})
set(eCAL_DATE             ${GIT_REVISION_DATE})


message(STATUS "eCAL version: ${eCAL_VERSION_STRING}")
message(DEBUG "eCAL git describe tag: ${GIT_DESCRIBE_TAG}")

include(helper_functions/ecal_add_functions)
include(helper_functions/ecal_helper_functions)
include(helper_functions/ecal_install_functions)

if(MSVC)
  set(eCAL_PLATFORM_TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET})
endif()

# --------------------------------------------------------
# offer the user the choice of overriding the installation directories
# --------------------------------------------------------
include(GNUInstallDirs)
set(eCAL_install_app_dir           ${CMAKE_INSTALL_BINDIR})
set(eCAL_install_archive_dir       ${CMAKE_INSTALL_LIBDIR})
set(eCAL_install_archive_dyn_dir   ${CMAKE_INSTALL_LIBDIR})
set(eCAL_install_bin_dir           ${CMAKE_INSTALL_BINDIR})
set(eCAL_install_config_dir        ${CMAKE_INSTALL_SYSCONFDIR}/ecal)
set(eCAL_install_cmake_dir         ${CMAKE_INSTALL_LIBDIR}/cmake/eCAL)
set(eCAL_install_doc_dir           ${CMAKE_INSTALL_DOCDIR})
set(eCAL_install_include_dir       ${CMAKE_INSTALL_INCLUDEDIR})
set(eCAL_install_lib_dir           ${CMAKE_INSTALL_LIBDIR})
set(eCAL_install_samples_dir       ${CMAKE_INSTALL_BINDIR})
set(eCAL_install_samples_src_dir   ${CMAKE_INSTALL_DATADIR}/ecal/samples/)
set(eCAL_install_tests_dir         ${CMAKE_INSTALL_BINDIR})

set(eCAL_config_dir                ${CMAKE_CURRENT_BINARY_DIR}/_generated/)
set(eCAL_config                    ${eCAL_config_dir}/eCALConfig.cmake)
set(eCAL_config_version            ${eCAL_config_dir}/eCALConfigVersion.cmake)

file(MAKE_DIRECTORY ${eCAL_config_dir})

if(WIN32)
  set(eCAL_install_config_dir            etc)
  set(eCAL_install_doc_dir               doc)
  set(eCAL_install_samples_dir           samples/bin/)
  set(eCAL_install_samples_src_dir       samples/src/)
  set(eCAL_install_tests_dir             tests)
endif()
set(BUILD_DEPLOY_DIRECTORY ${CMAKE_BINARY_DIR}/_deploy)

set(THREADS_PREFER_PTHREAD_FLAG ON)

if(WIN32)
  set(CMAKE_DEBUG_POSTFIX            d)
endif()

# --------------------------------------------------------
# ecal app protobuf interface
# --------------------------------------------------------
add_subdirectory(app/app_pb)

# --------------------------------------------------------
# ecal core
# --------------------------------------------------------
add_subdirectory(ecal)

# --------------------------------------------------------
# custom libs
# --------------------------------------------------------
add_subdirectory(lib/ThreadingUtils)
add_subdirectory(lib/CustomTclap)
add_subdirectory(lib/ecal_utils)

if(ECAL_USE_QT)
  add_subdirectory(lib/CustomQt)
endif()
add_subdirectory(lib/EcalParser)
if(ECAL_USE_QT)
  add_subdirectory(lib/QEcalParser)
endif()

# --------------------------------------------------------
# ecal mon plugin sdk
# --------------------------------------------------------
if(ECAL_USE_QT AND ECAL_BUILD_APP_SDK)
  add_subdirectory(app/mon/mon_plugin_lib)
endif()

# --------------------------------------------------------
# ecal rec addon sdk
# --------------------------------------------------------
if(ECAL_BUILD_APP_SDK)
  add_subdirectory(app/rec/rec_addon_core)
  add_subdirectory(app/rec/rec_addon_dummy)
endif()

# --------------------------------------------------------
# ecal time
# --------------------------------------------------------
if(ECAL_BUILD_TIMEPLUGINS)
  if(WIN32)
    set(ECAL_TIME_PLUGIN_DIR ecaltime_plugins)
  endif()
  add_subdirectory(contrib/ecaltime)
endif()

# --------------------------------------------------------
# ecal hdf5 support
# --------------------------------------------------------
if(ECAL_USE_HDF5)
  add_subdirectory(contrib/measurement/hdf5)
  add_subdirectory(contrib/ecalhdf5)
endif()
add_subdirectory(contrib/measurement/base)

# --------------------------------------------------------
# ecal core python binding
# --------------------------------------------------------
if(ECAL_BUILD_PY_BINDING)
  add_subdirectory(lang/python)
endif()

# --------------------------------------------------------
# ecal core csharp binding
# --------------------------------------------------------
if(ECAL_BUILD_CSHARP_BINDING AND WIN32)
  set(ECAL_CSHARP_BUILD_SAMPLES  ${ECAL_BUILD_SAMPLES})
  set(ECAL_CSHARP_BUILD_TESTS    ${ECAL_BUILD_TESTS})
  add_subdirectory(lang/csharp)
endif()

if(ECAL_BUILD_C_BINDING)
  set(ECAL_C_BUILD_SAMPLES  ${ECAL_BUILD_SAMPLES})
  set(ECAL_C_BUILD_TESTS    ${ECAL_BUILD_TESTS})
  add_subdirectory(lang/c)
endif()

# --------------------------------------------------------
# console applications
# --------------------------------------------------------
if(ECAL_BUILD_APPS)
  add_subdirectory(app/apps)
  if (ECAL_USE_FTXUI)
    add_subdirectory(app/mon/mon_tui)
  endif()
  add_subdirectory(app/mon/mon_cli)
  add_subdirectory(app/util/config)
  add_subdirectory(app/util/stop)
  add_subdirectory(app/sys/sys_core)
  add_subdirectory(app/sys/sys_cli)
  add_subdirectory(app/sys/sys_client_cli)
  add_subdirectory(app/sys/sys_client_core)
  if(NOT APPLE)
    add_subdirectory(app/mma)
  endif()
endif()

# --------------------------------------------------------
# applications using hdf5
# --------------------------------------------------------
if(ECAL_BUILD_APPS AND ECAL_USE_HDF5)
  add_subdirectory(app/play/play_core)
  add_subdirectory(app/play/play_cli)

  add_subdirectory(app/rec/rec_client_core)
  add_subdirectory(app/rec/rec_server_core)
  add_subdirectory(app/rec/rec_client_cli)
  add_subdirectory(app/rec/rec_server_cli)
  
  add_subdirectory(app/meas_cutter)
endif()

# --------------------------------------------------------
# qt applications
# --------------------------------------------------------
if(ECAL_BUILD_APPS AND (ECAL_USE_QT))
  add_subdirectory(app/sys/sys_gui)
  add_subdirectory(app/mon/mon_gui)
  if(WIN32)
    set(ECAL_MON_PLUGIN_DIR ecalmon_plugins)
  else()
    set(ECAL_MON_PLUGIN_DIR ecal/plugins/mon)
  endif()
  add_subdirectory(app/mon/mon_plugins)
  add_subdirectory(app/util/launcher)
  
  # --------------------------------------------------------
  # qt applications using hdf5
  # --------------------------------------------------------
  if(ECAL_USE_HDF5)
    add_subdirectory(app/play/play_gui)
    add_subdirectory(app/rec/rec_gui)
  endif()
endif()

# --------------------------------------------------------
# samples
# --------------------------------------------------------
if(ECAL_BUILD_SAMPLES)
  add_subdirectory(samples)
endif()


# --------------------------------------------------------
# Serialization extensions
# --------------------------------------------------------
add_subdirectory(serialization/common/common)

# Capnroto
if (ECAL_USE_CAPNPROTO)
set(ECAL_CAPNPROTO_BUILD_SAMPLES                                                                     ${ECAL_BUILD_SAMPLES})
set(ECAL_CAPNPROTO_BUILD_TESTS                                                                         ${ECAL_BUILD_TESTS})
add_subdirectory(serialization/capnproto)
endif ()

# Flatbuffers
if (ECAL_USE_FLATBUFFERS)
set(ECAL_FLATBUFFERS_BUILD_SAMPLES                                                                   ${ECAL_BUILD_SAMPLES})
set(ECAL_FLATBUFFERS_BUILD_TESTS                                                                       ${ECAL_BUILD_TESTS})
add_subdirectory(serialization/flatbuffers)
endif()

# Messagepack
#if (ECAL_USE_MESSAGEPACK)
#set(ECAL_MESSAGEPACK_BUILD_SAMPLES                                                                   ${ECAL_BUILD_SAMPLES})
#set(ECAL_MESSAGEPACK_BUILD_TESTS                                                                       ${ECAL_BUILD_TESTS})
#add_subdirectory(serialization/messagepack)
#endif()

# Protobuf
set(ECAL_PROTOBUF_BUILD_SAMPLES                                                                      ${ECAL_BUILD_SAMPLES})
set(ECAL_PROTOBUF_BUILD_TESTS                                                                          ${ECAL_BUILD_TESTS})
add_subdirectory(serialization/protobuf)

# String
set(ECAL_STRING_BUILD_SAMPLES                                                                        ${ECAL_BUILD_SAMPLES})
set(ECAL_STRING_BUILD_TESTS                                                                            ${ECAL_BUILD_TESTS})
add_subdirectory(serialization/string)



# --------------------------------------------------------
# unit tests
# --------------------------------------------------------
if(ECAL_BUILD_TESTS)
  enable_testing()

  # ------------------------------------------------------
  # test contrib
  # ------------------------------------------------------
  if(ECAL_USE_HDF5)
    add_subdirectory(tests/contrib/ecalhdf5/hdf5_test)
  endif()

  # ------------------------------------------------------
  # test apps
  # ------------------------------------------------------
  if (ECAL_USE_HDF5 AND ECAL_USE_QT)
    add_subdirectory(app/rec/rec_tests/rec_rpc_tests)
  endif()
endif()

if(ECAL_BUILD_DOCS)
  add_subdirectory(doc)
endif()

# --------------------------------------------------------
# debscripts
# --------------------------------------------------------
if (UNIX)
  add_subdirectory(cpack/debscripts)
endif()

#install(EXPORT eCALTargets DESTINATION cmake NAMESPACE eCAL::)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${eCAL_config_version}
  VERSION ${eCAL_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    cmake/${PROJECT_NAME}Config.cmake.in
    ${eCAL_config}
    INSTALL_DESTINATION ${${PROJECT_NAME}_install_cmake_dir}
    PATH_VARS ${PROJECT_NAME}_install_include_dir ${PROJECT_NAME}_install_cmake_dir
)

install(FILES ${eCAL_config} ${eCAL_config_version}
  DESTINATION ${eCAL_install_cmake_dir}
  COMPONENT sdk
)

install(FILES
  cmake/helper_functions/ecal_add_functions.cmake
  cmake/helper_functions/ecal_compiler_warnings.cmake
  cmake/helper_functions/ecal_helper_functions.cmake
  cmake/helper_functions/ecal_install_functions.cmake
  DESTINATION ${${PROJECT_NAME}_install_cmake_dir}/helper_functions
  COMPONENT sdk
)

install(EXPORT eCALCoreTargets
  FILE ${PROJECT_NAME}Targets.cmake
  DESTINATION ${eCAL_install_cmake_dir}
  NAMESPACE eCAL::
  COMPONENT sdk
)

if(ECAL_INSTALL_SAMPLE_SOURCES)
  install(DIRECTORY samples/
    DESTINATION ${eCAL_install_samples_src_dir}
  )
endif()

# --------------------------------------------------------
# create package
# --------------------------------------------------------
include(cpack/cpack_variables.cmake)

message(STATUS "Build Options:")
message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "CMAKE_EXPORT_COMPILE_COMMANDS                       : ${CMAKE_EXPORT_COMPILE_COMMANDS}")
message(STATUS "ECAL_USE_HDF5                                       : ${ECAL_USE_HDF5}")
message(STATUS "ECAL_USE_QT                                         : ${ECAL_USE_QT}")
message(STATUS "ECAL_USE_CURL                                       : ${ECAL_USE_CURL}")
message(STATUS "ECAL_USE_CAPNPROTO                                  : ${ECAL_USE_CAPNPROTO}")
message(STATUS "ECAL_USE_FLATBUFFERS                                : ${ECAL_USE_FLATBUFFERS}")
message(STATUS "ECAL_USE_FTXUI                                      : ${ECAL_USE_FTXUI}")
message(STATUS "BUILD_SHARED_LIBS                                   : ${BUILD_SHARED_LIBS}")
message(STATUS "ECAL_BUILD_DOCS                                     : ${ECAL_BUILD_DOCS}")
message(STATUS "ECAL_BUILD_DOCS_SAMPLES                             : ${ECAL_BUILD_DOCS_SAMPLES}")
message(STATUS "ECAL_BUILD_APPS                                     : ${ECAL_BUILD_APPS}")
message(STATUS "ECAL_BUILD_APP_SDK                                  : ${ECAL_BUILD_APP_SDK}")
message(STATUS "ECAL_BUILD_SAMPLES                                  : ${ECAL_BUILD_SAMPLES}")
message(STATUS "ECAL_BUILD_TIMEPLUGINS                              : ${ECAL_BUILD_TIMEPLUGINS}")
message(STATUS "ECAL_BUILD_C_BINDING                                : ${ECAL_BUILD_C_BINDING}")
message(STATUS "ECAL_BUILD_CSHARP_BINDING                           : ${ECAL_BUILD_CSHARP_BINDING}")
message(STATUS "ECAL_BUILD_PY_BINDING                               : ${ECAL_BUILD_PY_BINDING}")
message(STATUS "ECAL_BUILD_TESTS                                    : ${ECAL_BUILD_TESTS}")
message(STATUS "ECAL_INSTALL_SAMPLE_SOURCES                         : ${ECAL_INSTALL_SAMPLE_SOURCES}")
message(STATUS "ECAL_USE_NPCAP                                      : ${ECAL_USE_NPCAP}")
message(STATUS "ECAL_THIRDPARTY_BUILD_ASIO                          : ${ECAL_THIRDPARTY_BUILD_ASIO}")
message(STATUS "ECAL_THIRDPARTY_BUILD_ABSL                          : ${ECAL_THIRDPARTY_BUILD_ABSL}")
message(STATUS "ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS               : ${ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS}")
message(STATUS "ECAL_THIRDPARTY_BUILD_CURL                          : ${ECAL_THIRDPARTY_BUILD_CURL}")
message(STATUS "ECAL_THIRDPARTY_BUILD_FINEFTP                       : ${ECAL_THIRDPARTY_BUILD_FINEFTP}")
message(STATUS "ECAL_THIRDPARTY_BUILD_FTXUI                         : ${ECAL_THIRDPARTY_BUILD_FTXUI}")
message(STATUS "ECAL_THIRDPARTY_BUILD_GTEST                         : ${ECAL_THIRDPARTY_BUILD_GTEST}")
message(STATUS "ECAL_THIRDPARTY_BUILD_HDF5                          : ${ECAL_THIRDPARTY_BUILD_HDF5}")
message(STATUS "ECAL_THIRDPARTY_BUILD_NANOBIND                      : ${ECAL_THIRDPARTY_BUILD_NANOBIND}")
message(STATUS "ECAL_THIRDPARTY_BUILD_PROTOBUF                      : ${ECAL_THIRDPARTY_BUILD_PROTOBUF}")
message(STATUS "ECAL_THIRDPARTY_BUILD_QWT                           : ${ECAL_THIRDPARTY_BUILD_QWT}")
message(STATUS "ECAL_THIRDPARTY_BUILD_RECYCLE                       : ${ECAL_THIRDPARTY_BUILD_RECYCLE}")
message(STATUS "ECAL_THIRDPARTY_BUILD_SPDLOG                        : ${ECAL_THIRDPARTY_BUILD_SPDLOG}")
message(STATUS "ECAL_THIRDPARTY_BUILD_TCLAP                         : ${ECAL_THIRDPARTY_BUILD_TCLAP}")
message(STATUS "ECAL_THIRDPARTY_BUILD_TCP_PUBSUB                    : ${ECAL_THIRDPARTY_BUILD_TCP_PUBSUB}")
message(STATUS "ECAL_THIRDPARTY_BUILD_TERMCOLOR                     : ${ECAL_THIRDPARTY_BUILD_TERMCOLOR}")
message(STATUS "ECAL_THIRDPARTY_BUILD_TINYXML2                      : ${ECAL_THIRDPARTY_BUILD_TINYXML2}")
message(STATUS "ECAL_THIRDPARTY_BUILD_UDPCAP                        : ${ECAL_THIRDPARTY_BUILD_UDPCAP}")
message(STATUS "ECAL_THIRDPARTY_BUILD_YAML-CPP                      : ${ECAL_THIRDPARTY_BUILD_YAML-CPP}")
message(STATUS "ECAL_LINK_HDF5_SHARED                               : ${ECAL_LINK_HDF5_SHARED}")
message(STATUS "ECAL_CPACK_PACK_WITH_INNOSETUP                      : ${ECAL_CPACK_PACK_WITH_INNOSETUP}")
message(STATUS "--------------------------------------------------------------------------------")
