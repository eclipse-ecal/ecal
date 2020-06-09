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

cmake_minimum_required(VERSION 3.13)

list(APPEND CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

project(eCAL)

# Set CMake policy behavior (alias targets)
cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0054 NEW)
#cmake_policy(SET CMP0077 NEW) // currently not working with older CMake version

# --------------------------------------------------------
# command line build options
# use it that way cmake .. -DBUILD_APPS=ON -DBUILD_SAMPLES=ON
# --------------------------------------------------------
option(HAS_HDF5                                "Platform supports HDF5 library"                                    ON)
option(HAS_QT5                                 "Platform supports Qt 5 library"                                    ON)
option(HAS_CAPNPROTO                           "Platform supports Cap'n Proto library"                            OFF)

option(BUILD_DOCS                              "Build the eCAL documentation"                                     OFF)
option(BUILD_APPS                              "Build the eCAL applications"                                       ON)
option(BUILD_SAMPLES                           "Build the eCAL samples"                                            ON)
option(BUILD_TIME                              "Build the eCAL time interfaces"                                    ON)
option(BUILD_PY_BINDING                        "Build eCAL python binding"                                        OFF)
option(BUILD_CSHARP_BINDING                    "Build eCAL C# binding"                                            OFF)
option(BUILD_TESTS                             "Build the eCAL google tests"                                      OFF)

option(ECAL_LAYER_ICEORYX                      "Use iceoryx shared memory as local communication layer"           OFF)

option(ECAL_INCLUDE_PY_SAMPLES                 "Include python language sample projects into CMake"               OFF)
option(ECAL_INSTALL_SAMPLE_SOURCES             "Install the sources of eCAL samples"                               ON)

option(ECAL_JOIN_MULTICAST_TWICE               "Specific Multicast Network Bug Workaround"                        OFF)
option(ECAL_NPCAP_SUPPORT                      "Enable the eCAL Npcap Receiver (i.e. the Win10 performance fix)"  OFF)

# Set option regarding third party library builds
option(ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS   "Build CMakeFunctions with eCAL"                                    ON)
option(ECAL_THIRDPARTY_BUILD_PROTOBUF          "Build protobuf with eCAL"                                          ON)
option(ECAL_THIRDPARTY_BUILD_SPDLOG            "Build spdlog with eCAL"                                            ON)
option(ECAL_THIRDPARTY_BUILD_GTEST             "Build gtest with eCAL"                                            OFF)

set(ECAL_INSTALL_PYTHON_DIR "bin" CACHE PATH "Location to install the Python extension modules. Might be set by setupdtools install.")

# there is a CMake issue with testing threading availibility via TEST_RUN
if(${CMAKE_CROSSCOMPILING})
  set(THREADS_PTHREAD_ARG "2" CACHE STRING "Forcibly set by CMakeLists.txt." FORCE)
endif()


# find_project(eCAL calls shall be ignored, eCAL is build as part of the project)
set(as_subproject eCAL)

# Normalize backslashes from Windows paths
file(TO_CMAKE_PATH "${CMAKE_MODULE_PATH}" CMAKE_MODULE_PATH)
file(TO_CMAKE_PATH "${CMAKE_PREFIX_PATH}" CMAKE_PREFIX_PATH)
message(STATUS "Module Path: ${CMAKE_MODULE_PATH}")
message(STATUS "Prefix Path: ${CMAKE_PREFIX_PATH}")

# --------------------------------------------------------
# check options
# --------------------------------------------------------
if(WIN32 AND ECAL_LAYER_ICEORYX)
  message(FATAL_ERROR "Iceoryx transport layer not available for windows os. \
                       Please disable the layer by setting ECAL_LAYER_ICEORYX to OFF.")
endif()

# --------------------------------------------------------
# third party dependencies as requested
# --------------------------------------------------------
if (ECAL_THIRDPARTY_BUILD_PROTOBUF OR ECAL_THIRDPARTY_BUILD_SPDLOG OR ECAL_THIRDPARTY_BUILD_GTEST)
  if(MSVC)
    # supress thirdparty warnings for windows platform
    set(CMAKE_CXX_FLAGS_OLD "${CMAKE_CXX_FLAGS}")
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W0" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
    endif()
  endif()
endif()

if (ECAL_THIRDPARTY_BUILD_PROTOBUF)
  list(APPEND as_subproject Protobuf)
  set(protobuf_BUILD_TESTS OFF CACHE BOOL "My option" FORCE)
  set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "My option" FORCE)
  if(UNIX)
    set(protobuf_BUILD_SHARED_LIBS ON CACHE BOOL "My option" FORCE)
  endif()
  add_subdirectory(thirdparty/protobuf/cmake)
  if (NOT TARGET protobuf::libprotobuf)
    add_library(protobuf::libprotobuf ALIAS libprotobuf)
  endif()
  if (NOT TARGET protobuf::protoc)
    add_executable(protobuf::protoc ALIAS protoc)
    set(Protobuf_PROTOC_EXECUTABLE protoc)
  endif()
  set(Protobuf_VERSION_MAJOR 3)
  set(Protobuf_VERSION_MINOR 11)
  set(Protobuf_VERSION_PATCH 4)
endif()

if (ECAL_THIRDPARTY_BUILD_SPDLOG)
  list(APPEND as_subproject spdlog)
  set(SPDLOG_BUILD_TESTING OFF CACHE BOOL "My option" FORCE)
  add_subdirectory(thirdparty/spdlog)
  add_library(spdlog::spdlog ALIAS spdlog)
endif ()

if (ECAL_THIRDPARTY_BUILD_GTEST)
  list(APPEND as_subproject GTest)
  # Googletest automatically forces MT instead of MD if we do not set this option.
  if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "My option" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "My option" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "My option" FORCE)
  endif()
  add_subdirectory(thirdparty/googletest)
  if(NOT TARGET GTest::GTest)
    add_library(GTest::GTest ALIAS gtest)
  endif()
  if(NOT TARGET GTest::Main)
    add_library(GTest::Main ALIAS gtest_main)
  endif()
endif()

if (ECAL_THIRDPARTY_BUILD_PROTOBUF OR ECAL_THIRDPARTY_BUILD_SPDLOG OR ECAL_THIRDPARTY_BUILD_GTEST)
  if(MSVC)
    # reset thirdparty warnings for windows platform
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_OLD}")
  endif()
endif()

if (ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS)
  list(APPEND as_subproject CMakeFunctions)
endif()

macro(ecal_find_package)
  if(NOT "${ARGV0}" IN_LIST as_subproject)
    find_package(${ARGV})
  endif()
endmacro()

if (ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS)
  add_subdirectory(thirdparty/cmake_functions)
endif ()



ecal_find_package(CMakeFunctions REQUIRED)

git_revision_information()
set(eCAL_VERSION_MAJOR  ${GIT_REVISION_MAYOR})
set(eCAL_VERSION_MINOR  ${GIT_REVISION_MINOR})
set(eCAL_VERSION_PATCH  ${GIT_REVISION_PATCH})
set(eCAL_VERSION_STRING ${eCAL_VERSION_MAJOR}.${eCAL_VERSION_MINOR}.${eCAL_VERSION_PATCH})
set(eCAL_VERSION        ${eCAL_VERSION_STRING})

include(helper_functions/ecal_add_functions)
include(helper_functions/ecal_helper_functions)
include(helper_functions/ecal_install_functions)
include(helper_functions/ecal_python_functions)
include(build_location)

if(MSVC)
  set(eCAL_PLATFORM_TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET})
endif()

# --------------------------------------------------------
# detect qt library
# --------------------------------------------------------
if(WIN32 AND HAS_QT5)
  if (NOT DEFINED ENV{QT5_ROOT_DIRECTORY} AND NOT DEFINED ENV{QT5_ROOT})
    # If the QT5_ROOT_DIRECTORY Variable is not set, there is still a chance that
    # the user set the CMAKE_PREFIX_PATH himself. Thus we try to find
    # Qt5 Core just to see if that works. If we can find Qt, we assume
    # that the user knows what he is doing.
    ecal_find_package(Qt5 COMPONENTS Core)
    if(NOT Qt5_FOUND)
      message(FATAL_ERROR "QT5_ROOT_DIRECTORY is not defined. Please create an \
        environment variable QT5_ROOT_DIRECTORY pointing to your QT5 installation \
        (the directory that contains the msvc201x or msvc201x_64 \
        directory) or disable compilation of Qt5 projects by setting \
        HAS_QT5 to OFF.")
    endif()
  else()
    if (DEFINED ENV{QT5_ROOT})
      set(qt_directory $ENV{QT5_ROOT})
      message(WARNING "Please unset the QT5_ROOT environment variable and use \
        QT5_ROOT_DIRECTORY instead. Using QT5_ROOT will trigger multiple CMake warnings \
        in newer CMake versions.")
    endif ()
    if (DEFINED ENV{QT5_ROOT_DIRECTORY})
      set(qt_directory $ENV{QT5_ROOT_DIRECTORY})
    endif ()

    qt_msvc_path(qt_subfolder)
    list(APPEND CMAKE_PREFIX_PATH ${qt_directory}/${qt_subfolder}/)
    message(STATUS "Searching for QT in ${qt_directory}/${qt_subfolder}/")
  endif()
endif()

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
# set mingw specific options
# --------------------------------------------------------
if(MINGW)
  add_definitions(-std=c++14 -DWINVER=0x0601)
endif()

# --------------------------------------------------------
# set gcc specific options
# --------------------------------------------------------
if(UNIX)
  message(STATUS "GCC detected - Adding flags")
  set(CMAKE_CXX_STANDARD 14)
  add_definitions(-Wall -Wextra -std=c++14)
endif()

# --------------------------------------------------------
# set install prefix
# --------------------------------------------------------
if(WIN32)
  if(NOT ONCE_SET_CMAKE_INSTALL_PREFIX)
    set(ONCE_SET_CMAKE_INSTALL_PREFIX TRUE CACHE BOOL
      "Have we set the install prefix yet?" FORCE)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/_install CACHE PATH
      "Install path prefix, prepended onto install directories"
      FORCE)
  endif(NOT ONCE_SET_CMAKE_INSTALL_PREFIX)
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
set(eCAL_install_include_dir       ${CMAKE_INSTALL_INCLUDEDIR})
set(eCAL_install_lib_dir           ${CMAKE_INSTALL_LIBDIR})
set(eCAL_install_samples_dir       ${CMAKE_INSTALL_BINDIR})
set(eCAL_install_samples_src_dir   ${CMAKE_INSTALL_DATADIR}/ecal/samples/)
set(eCAL_install_tests_dir         ${CMAKE_INSTALL_BINDIR})

set(eCAL_config_dir                ${eCAL_BINARY_DIR}/cmake/)
set(eCAL_config                    ${eCAL_config_dir}/eCALConfig.cmake)
set(eCAL_config_version            ${eCAL_config_dir}/eCALConfigVersion.cmake)
if(WIN32)
  set(eCAL_install_config_dir            cfg)
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
# protobuf utilities
# --------------------------------------------------------
add_subdirectory(contrib/ecalproto)

# --------------------------------------------------------
# ecal core internal protobuf
# --------------------------------------------------------
add_subdirectory(ecal/pb)

# --------------------------------------------------------
# ecal core
# --------------------------------------------------------
add_subdirectory(ecal/core)

# --------------------------------------------------------
# custom libs
# --------------------------------------------------------
add_subdirectory(lib/ThreadingUtils)
add_subdirectory(lib/CustomTclap)
add_subdirectory(lib/ecal_utils)
if(HAS_QT5)
  add_subdirectory(lib/CustomQt)
endif()
if(ECAL_NPCAP_SUPPORT)
  add_subdirectory(lib/Udpcap)
endif()
add_subdirectory(lib/EcalParser)
if(HAS_QT5)
  add_subdirectory(lib/QEcalParser)
endif(HAS_QT5)

# --------------------------------------------------------
# ecal mon plugin sdk
# --------------------------------------------------------
add_subdirectory(app/mon/mon_plugin_lib)

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
  add_subdirectory(contrib/ecalhdf5)
  add_subdirectory(contrib/message)
endif()

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
  add_subdirectory(lang/csharp/Continental/eCAL/Core/)
  add_subdirectory(lang/csharp/Continental/eCAL/Protobuf/)
endif()

# --------------------------------------------------------
# console applications
# --------------------------------------------------------
add_subdirectory(app/apps)
if(BUILD_APPS)
  add_subdirectory(app/mon/mon_cli)
  add_subdirectory(app/util/config)
  add_subdirectory(app/util/stop)
endif()

# --------------------------------------------------------
# applications using hdf5
# --------------------------------------------------------
if(BUILD_APPS AND HAS_HDF5)
  add_subdirectory(app/play/play_core)
  add_subdirectory(app/play/play_cli)

  add_subdirectory(app/rec/rec_cli)
  add_subdirectory(app/rec/rec_core)
  add_subdirectory(app/rec/rec_server_core)
endif()

# --------------------------------------------------------
# qt applications
# --------------------------------------------------------
if(BUILD_APPS AND HAS_QT5)
  add_subdirectory(app/mon/mon_gui)
  add_subdirectory(app/util/launcher)
  if(WIN32)
    set(ECAL_MON_PLUGIN_DIR ecalmon_plugins)
  else()
    set(ECAL_MON_PLUGIN_DIR ecal/plugins/mon)
  endif()
  add_subdirectory(app/mon/mon_plugins)
# --------------------------------------------------------
# qt applications using hdf5
# --------------------------------------------------------
if(BUILD_APPS AND HAS_QT5 AND HAS_HDF5)
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
if(BUILD_TESTS)
  enable_testing()

  # ------------------------------------------------------
  # test contrib
  # ------------------------------------------------------
  if(HAS_HDF5)
    add_subdirectory(testing/contrib/ecalhdf5/hdf5_test)
  endif()
  add_subdirectory(testing/contrib/ecalproto/dynproto_test)
  add_subdirectory(testing/contrib/ecalproto/ecal_proto_test)

  # ------------------------------------------------------
  # test ecal
  # ------------------------------------------------------
  add_subdirectory(testing/ecal/clientserver_test)
  add_subdirectory(testing/ecal/core_test)
  add_subdirectory(testing/ecal/event_test)
  add_subdirectory(testing/ecal/expmap_test)
if(NOT ECAL_LAYER_ICEORYX)
  add_subdirectory(testing/ecal/io_memfile_test)
endif(NOT ECAL_LAYER_ICEORYX)
  add_subdirectory(testing/ecal/pubsub_inproc_test)
  add_subdirectory(testing/ecal/pubsub_proto_test)
  add_subdirectory(testing/ecal/pubsub_test)
endif()

# --------------------------------------------------------
# configs
# --------------------------------------------------------
add_subdirectory(ecal/core/cfg)

if(BUILD_DOCS)
  add_subdirectory(doc)
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
