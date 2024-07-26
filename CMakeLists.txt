# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
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

# --------------------------------------------------------
# command line build options
# use it that way cmake .. -DBUILD_APPS=ON -DBUILD_SAMPLES=ON
# --------------------------------------------------------
option(HAS_HDF5                                "Platform supports HDF5 library"                                    ON)

# If the user set the legacy HAS_QT5 option, but didn't set the new HAS_QT option, set it to the value of HAS_QT5
if(DEFINED HAS_QT5 AND NOT DEFINED HAS_QT)
  set(HAS_QT ${HAS_QT5})
  message(WARNING "The option HAS_QT5 is deprecated and may be removed at any time. Please use HAS_QT instead.")
endif()

option(HAS_QT                                  "Platform supports Qt 5 / 6 library."                               ON)

option(HAS_CURL                                "Build with CURL (i.e. upload support in the recorder app)"         ON)
option(HAS_CAPNPROTO                           "Platform supports Cap'n Proto library"                            OFF)
option(HAS_FLATBUFFERS                         "Platform supports flatbuffers library"                            OFF)
option(HAS_FTXUI                               "Platform supports FTXUI library. Requires C++17 and up."           ON)

option(BUILD_DOCS                              "Build the eCAL documentation"                                     OFF)
option(BUILD_APPS                              "Build the eCAL applications"                                       ON)
option(BUILD_SAMPLES                           "Build the eCAL samples"                                            ON)
option(BUILD_TIME                              "Build the eCAL time interfaces"                                    ON)
option(BUILD_PY_BINDING                        "Build eCAL python binding"                                        OFF)
option(BUILD_CSHARP_BINDING                    "Build eCAL C# binding"                                            OFF)
option(BUILD_ECAL_TESTS                        "Build the eCAL google tests"                                      OFF)

option(ECAL_INCLUDE_PY_SAMPLES                 "Include python language sample projects into CMake"               OFF)
option(ECAL_INSTALL_SAMPLE_SOURCES             "Install the sources of eCAL samples"                               ON)

option(ECAL_NPCAP_SUPPORT                      "Enable the eCAL Npcap Receiver (i.e. the Win10 performance fix)"  OFF)
option(ECAL_USE_CLOCKLOCK_MUTEX                "Use native mutex with monotonic clock (requires glibc >= 2.30)"   OFF)

# --------------------------------------------------------
# ecal core configuration
# --------------------------------------------------------
set(ECAL_CORE_HAS_PROTOBUF                                                                                         ON)
set(ECAL_CORE_BUILD_SAMPLES                                                                          ${BUILD_SAMPLES})
set(ECAL_CORE_BUILD_TESTS                                                                         ${BUILD_ECAL_TESTS})
set(ECAL_CORE_CONFIGURATION                                                                                        ON)
set(ECAL_CORE_COMMAND_LINE                                                                                         ON)
set(ECAL_CORE_REGISTRATION                                                                                         ON)
set(ECAL_CORE_MONITORING                                                                                           ON)
set(ECAL_CORE_PUBLISHER                                                                                            ON)
set(ECAL_CORE_SUBSCRIBER                                                                                           ON)
set(ECAL_CORE_SERVICE                                                                                              ON)
set(ECAL_CORE_TIMEPLUGIN                                                                                           ON)
set(ECAL_CORE_NPCAP_SUPPORT                                                                     ${ECAL_NPCAP_SUPPORT})
set(ECAL_CORE_REGISTRATION_SHM                                                                                     ON)
set(ECAL_CORE_TRANSPORT_UDP                                                                                        ON)
set(ECAL_CORE_TRANSPORT_TCP                                                                                        ON)
set(ECAL_CORE_TRANSPORT_SHM                                                                                        ON)

# This should be ON, unless build ecal hdf5 for Matlab usage
option(ECAL_LINK_HDF5_SHARED                   "Link shared libs of HDF5"                                          ON)

option(CPACK_PACK_WITH_INNOSETUP               "Create Innosetup installer for the Windows build"                  ON)

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

set(ECAL_CSHARP_BUILD_SAMPLES  ${BUILD_SAMPLES})


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

# --------------------------------------------------------
# detect qt library
# --------------------------------------------------------
if(MSVC)
  if (HAS_QT)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Core QUIET)
    if (NOT "${QT_FOUND}")
	  include("cmake/qt_msvc_path.cmake")
      autodetect_qt_msvc_dir()
    endif()
  endif()
endif()

find_package(CMakeFunctions REQUIRED)

git_revision_information(DEFAULT ${ECAL_BUILD_VERSION})
set(eCAL_VERSION_MAJOR  ${GIT_REVISION_MAJOR})
set(eCAL_VERSION_MINOR  ${GIT_REVISION_MINOR})
set(eCAL_VERSION_PATCH  ${GIT_REVISION_PATCH})
set(eCAL_VERSION_STRING ${eCAL_VERSION_MAJOR}.${eCAL_VERSION_MINOR}.${eCAL_VERSION_PATCH})
set(eCAL_VERSION        ${eCAL_VERSION_STRING})

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
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
  "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/eCAL)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for CMake files")

include(GNUInstallDirs)
set(eCAL_install_app_dir           ${CMAKE_INSTALL_BINDIR})
set(eCAL_install_archive_dir       ${CMAKE_INSTALL_LIBDIR})
set(eCAL_install_archive_dyn_dir   ${CMAKE_INSTALL_LIBDIR})
set(eCAL_install_bin_dir           ${CMAKE_INSTALL_BINDIR})
set(eCAL_install_cmake_dir         ${CMAKE_INSTALL_LIBDIR}/cmake/eCAL)
set(eCAL_install_config_dir        ${CMAKE_INSTALL_SYSCONFDIR}/ecal)
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
  set(eCAL_install_config_dir            cfg)
  set(eCAL_install_doc_dir               doc)
  set(eCAL_install_samples_dir           samples/bin/)
  set(eCAL_install_samples_src_dir       samples/src/)
  set(eCAL_install_tests_dir             tests)
endif()
set(BUILD_DEPLOY_DIRECTORY ${CMAKE_BINARY_DIR}/_deploy)

set(THREADS_PREFER_PTHREAD_FLAG ON)

if(WIN32)
  set(CMAKE_DEBUG_POSTFIX            d)
  set(CMAKE_MINSIZEREL_POSTFIX       minsize)
  set(CMAKE_RELWITHDEBINFO_POSTFIX   reldbg)
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

if(HAS_QT)
  add_subdirectory(lib/CustomQt)
endif()
add_subdirectory(lib/EcalParser)
if(HAS_QT)
  add_subdirectory(lib/QEcalParser)
endif()

# --------------------------------------------------------
# ecal mon plugin sdk
# --------------------------------------------------------
if(HAS_QT)
  add_subdirectory(app/mon/mon_plugin_lib)
endif()

# --------------------------------------------------------
# ecal rec addon sdk
# --------------------------------------------------------
add_subdirectory(app/rec/rec_addon_core)
add_subdirectory(app/rec/rec_addon_dummy)

# --------------------------------------------------------
# ecal time
# --------------------------------------------------------
if(BUILD_TIME)
  if(WIN32)
    set(ECAL_TIME_PLUGIN_DIR ecaltime_plugins)
  endif()
  add_subdirectory(contrib/ecaltime)
endif()

# --------------------------------------------------------
# ecal hdf5 support
# --------------------------------------------------------
if(HAS_HDF5)
  add_subdirectory(contrib/measurement/hdf5)
  add_subdirectory(contrib/ecalhdf5)
  add_subdirectory(contrib/message)
endif()
add_subdirectory(contrib/measurement/base)

# --------------------------------------------------------
# ecal core python binding
# --------------------------------------------------------
if(BUILD_PY_BINDING)
  add_subdirectory(lang/python)
endif()

# --------------------------------------------------------
# ecal core csharp binding
# --------------------------------------------------------
if(BUILD_CSHARP_BINDING AND WIN32)
  add_subdirectory(lang/csharp)
endif()

# --------------------------------------------------------
# console applications
# --------------------------------------------------------
add_subdirectory(app/apps)
if(BUILD_APPS)
  if (HAS_FTXUI)
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
    add_subdirectory(contrib/mma)
  endif()
endif()

# --------------------------------------------------------
# applications using hdf5
# --------------------------------------------------------
if(BUILD_APPS AND HAS_HDF5)
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
if(BUILD_APPS AND (HAS_QT))
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
  if(HAS_HDF5)
    add_subdirectory(app/play/play_gui)
    add_subdirectory(app/rec/rec_gui)
  endif()
endif()

# --------------------------------------------------------
# samples
# --------------------------------------------------------
if(BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

# --------------------------------------------------------
# unit tests
# --------------------------------------------------------
if(BUILD_ECAL_TESTS)
  enable_testing()

  # ------------------------------------------------------
  # test contrib
  # ------------------------------------------------------
  if(HAS_HDF5)
    add_subdirectory(tests/contrib/ecalhdf5/hdf5_test)
  endif()
  add_subdirectory(tests/contrib/ecalproto/dynproto_test)
  add_subdirectory(tests/contrib/ecalproto/ecal_proto_test)

  # ------------------------------------------------------
  # test apps
  # ------------------------------------------------------
  if (HAS_HDF5 AND HAS_QT)
    add_subdirectory(app/rec/rec_tests/rec_rpc_tests)
  endif()
endif()

if(BUILD_DOCS)
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
message(STATUS "CMAKE_EXPORT_COMPILE_COMMANDS                  : ${CMAKE_EXPORT_COMPILE_COMMANDS}")
message(STATUS "HAS_HDF5                                       : ${HAS_HDF5}")
message(STATUS "HAS_QT                                         : ${HAS_QT}")
message(STATUS "HAS_CURL                                       : ${HAS_CURL}")
message(STATUS "HAS_CAPNPROTO                                  : ${HAS_CAPNPROTO}")
message(STATUS "HAS_FLATBUFFERS                                : ${HAS_FLATBUFFERS}")
message(STATUS "HAS_FTXUI                                      : ${HAS_FTXUI}")
message(STATUS "BUILD_SHARED_LIBS                              : ${BUILD_SHARED_LIBS}")
message(STATUS "BUILD_DOCS                                     : ${BUILD_DOCS}")
message(STATUS "BUILD_APPS                                     : ${BUILD_APPS}")
message(STATUS "BUILD_SAMPLES                                  : ${BUILD_SAMPLES}")
message(STATUS "BUILD_TIME                                     : ${BUILD_TIME}")
message(STATUS "BUILD_PY_BINDING                               : ${BUILD_PY_BINDING}")
message(STATUS "BUILD_CSHARP_BINDING                           : ${BUILD_CSHARP_BINDING}")
message(STATUS "BUILD_ECAL_TESTS                               : ${BUILD_ECAL_TESTS}")
message(STATUS "ECAL_INCLUDE_PY_SAMPLES                        : ${ECAL_INCLUDE_PY_SAMPLES}")
message(STATUS "ECAL_INSTALL_SAMPLE_SOURCES                    : ${ECAL_INSTALL_SAMPLE_SOURCES}")
message(STATUS "ECAL_NPCAP_SUPPORT                             : ${ECAL_NPCAP_SUPPORT}")
message(STATUS "ECAL_THIRDPARTY_BUILD_ASIO                     : ${ECAL_THIRDPARTY_BUILD_ASIO}")
message(STATUS "ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS          : ${ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS}")
message(STATUS "ECAL_THIRDPARTY_BUILD_CURL                     : ${ECAL_THIRDPARTY_BUILD_CURL}")
message(STATUS "ECAL_THIRDPARTY_BUILD_FINEFTP                  : ${ECAL_THIRDPARTY_BUILD_FINEFTP}")
message(STATUS "ECAL_THIRDPARTY_BUILD_FTXUI                    : ${ECAL_THIRDPARTY_BUILD_FTXUI}")
message(STATUS "ECAL_THIRDPARTY_BUILD_GTEST                    : ${ECAL_THIRDPARTY_BUILD_GTEST}")
message(STATUS "ECAL_THIRDPARTY_BUILD_HDF5                     : ${ECAL_THIRDPARTY_BUILD_HDF5}")
message(STATUS "ECAL_THIRDPARTY_BUILD_PROTOBUF                 : ${ECAL_THIRDPARTY_BUILD_PROTOBUF}")
message(STATUS "ECAL_THIRDPARTY_BUILD_QWT                      : ${ECAL_THIRDPARTY_BUILD_QWT}")
message(STATUS "ECAL_THIRDPARTY_BUILD_RECYCLE                  : ${ECAL_THIRDPARTY_BUILD_RECYCLE}")
message(STATUS "ECAL_THIRDPARTY_BUILD_SIMPLEINI                : ${ECAL_THIRDPARTY_BUILD_SIMPLEINI}")
message(STATUS "ECAL_THIRDPARTY_BUILD_SPDLOG                   : ${ECAL_THIRDPARTY_BUILD_SPDLOG}")
message(STATUS "ECAL_THIRDPARTY_BUILD_TCLAP                    : ${ECAL_THIRDPARTY_BUILD_TCLAP}")
message(STATUS "ECAL_THIRDPARTY_BUILD_TCP_PUBSUB               : ${ECAL_THIRDPARTY_BUILD_TCP_PUBSUB}")
message(STATUS "ECAL_THIRDPARTY_BUILD_TERMCOLOR                : ${ECAL_THIRDPARTY_BUILD_TERMCOLOR}")
message(STATUS "ECAL_THIRDPARTY_BUILD_TINYXML2                 : ${ECAL_THIRDPARTY_BUILD_TINYXML2}")
message(STATUS "ECAL_THIRDPARTY_BUILD_UDPCAP                   : ${ECAL_THIRDPARTY_BUILD_UDPCAP}")
message(STATUS "ECAL_THIRDPARTY_BUILD_YAML-CPP                 : ${ECAL_THIRDPARTY_BUILD_YAML-CPP}")
message(STATUS "ECAL_LINK_HDF5_SHARED                          : ${ECAL_LINK_HDF5_SHARED}")
message(STATUS "CPACK_PACK_WITH_INNOSETUP                      : ${CPACK_PACK_WITH_INNOSETUP}")
message(STATUS "--------------------------------------------------------------------------------")
