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
# Attention: When building HDF5, the minimum required version becomes 3.14.
# Reason for at least 3.14: We are using CPM (CMake Package Manager), which is
# an extension for CMake's native FetchContent module. FetchContent was extended
# in CMake 3.14 and CPM needs those improvements.

if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW) # Prevent HDF5 from overwriting our output paths
endif (POLICY CMP0077)

list(APPEND CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

project(eCAL)
set(ECAL_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}")

# --------------------------------------------------------
# command line build options
# use it that way cmake .. -DBUILD_APPS=ON -DBUILD_SAMPLES=ON
# --------------------------------------------------------
option(HAS_HDF5                                "Platform supports HDF5 library"                                    ON)
option(HAS_QT5                                 "Platform supports Qt 5 library"                                    ON)
option(HAS_CURL                                "Build with CURL (i.e. upload support in the recorder app)"         ON)
option(HAS_CAPNPROTO                           "Platform supports Cap'n Proto library"                            OFF)

option(BUILD_DOCS                              "Build the eCAL documentation"                                     OFF)
option(BUILD_APPS                              "Build the eCAL applications"                                       ON)
option(BUILD_SAMPLES                           "Build the eCAL samples"                                            ON)
option(BUILD_TIME                              "Build the eCAL time interfaces"                                    ON)
option(BUILD_PY_BINDING                        "Build eCAL python binding"                                        OFF)
option(BUILD_CSHARP_BINDING                    "Build eCAL C# binding"                                            OFF)
option(BUILD_ECAL_TESTS                        "Build the eCAL google tests"                                      OFF)

option(ECAL_LAYER_ICEORYX                      "Use iceoryx shared memory as local communication layer"           OFF)

option(ECAL_INCLUDE_PY_SAMPLES                 "Include python language sample projects into CMake"               OFF)
option(ECAL_INSTALL_SAMPLE_SOURCES             "Install the sources of eCAL samples"                               ON)

option(ECAL_JOIN_MULTICAST_TWICE               "Specific Multicast Network Bug Workaround"                        OFF)
option(ECAL_NPCAP_SUPPORT                      "Enable the eCAL Npcap Receiver (i.e. the Win10 performance fix)"  OFF)

# Set option regarding third party library builds
option(ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS   "Build CMakeFunctions with eCAL"                                    ON)
option(ECAL_THIRDPARTY_BUILD_SPDLOG            "Build spdlog with eCAL"                                            ON)
option(ECAL_THIRDPARTY_BUILD_TINYXML2          "Build tinyxml2 with eCAL"                                          ON)
option(ECAL_THIRDPARTY_BUILD_FINEFTP           "Build fineFTP with eCAL"                                           ON)
option(ECAL_THIRDPARTY_BUILD_TERMCOLOR         "Build termcolor with eCAL"                                         ON)
# option(ECAL_THIRDPARTY_BUILD_ZLIB              "Build zlib with eCAL"                                              ON)
# option(ECAL_THIRDPARTY_BUILD_LIBSSH2           "Build libssh2 with eCAL"                                           ON)
option(ECAL_THIRDPARTY_BUILD_GTEST             "Build gtest with eCAL"                                            OFF)

if(WIN32)
  option(ECAL_THIRDPARTY_BUILD_PROTOBUF        "Build protobuf with eCAL"                                          ON)
  option(ECAL_THIRDPARTY_BUILD_CURL            "Build CURL with eCAL"                                              ON)
  option(ECAL_THIRDPARTY_BUILD_HDF5            "Build HDF5 with eCAL"                                              ON)
else()
  option(ECAL_THIRDPARTY_BUILD_PROTOBUF        "Build protobuf with eCAL"                                          OFF)
  option(ECAL_THIRDPARTY_BUILD_CURL            "Build CURL with eCAL"                                              OFF)
  option(ECAL_THIRDPARTY_BUILD_HDF5            "Build HDF5 with eCAL"                                              OFF)
endif()

# This should be ON, unless build ecal hdf5 for Matlab usage
option(ECAL_LINK_HDF5_SHARED                   "Link shared libs of HDF5"                                          ON)

option(CPACK_PACK_WITH_INNOSETUP               "Create Innosetup installer for the Windows build"                  ON)

set(ECAL_INSTALL_PYTHON_DIR "bin" CACHE PATH "Location to install the Python extension modules. Might be set by setupdtools install.")

set(ECAL_BUILD_VERSION "0.0.0" CACHE STRING "Inject a build version if not building from a git repository")

# there is a CMake issue with testing threading availibility via TEST_RUN
if(${CMAKE_CROSSCOMPILING})
  set(THREADS_PTHREAD_ARG "2" CACHE STRING "Forcibly set by CMakeLists.txt." FORCE)
endif()


include(build_location)

# find_project(eCAL calls shall be ignored, eCAL is build as part of the project)
set(as_subproject eCAL)

# Normalize backslashes from Windows paths
file(TO_CMAKE_PATH "${CMAKE_MODULE_PATH}" CMAKE_MODULE_PATH)
file(TO_CMAKE_PATH "${CMAKE_PREFIX_PATH}" CMAKE_PREFIX_PATH)
message(STATUS "Module Path: ${CMAKE_MODULE_PATH}")
message(STATUS "Prefix Path: ${CMAKE_PREFIX_PATH}")

# --------------------------------------------------------
# third party dependencies as requested
# --------------------------------------------------------
if (ECAL_THIRDPARTY_BUILD_PROTOBUF OR
    ECAL_THIRDPARTY_BUILD_SPDLOG OR
    ECAL_THIRDPARTY_BUILD_TINYXML2 OR
    ECAL_THIRDPARTY_BUILD_FINEFTP OR
    ECAL_THIRDPARTY_BUILD_ZLIB OR
    ECAL_THIRDPARTY_BUILD_LIBSSH2 OR
    ECAL_THIRDPARTY_BUILD_CURL OR
    ECAL_THIRDPARTY_BUILD_GTEST OR
    ECAL_THIRDPARTY_BUILD_HDF5
    )
  if(MSVC)
    message(STATUS "supress thirdparty warnings for windows platform ..")
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

  get_target_property(is_imported libprotobuf IMPORTED)
  if (NOT is_imported)
    # Disable warnings for third-party lib
    target_compile_options(libprotobuf PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/W0>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
    )
  endif()

  if (NOT TARGET protoc AND CMAKE_CROSSCOMPILING)
    # In cross compile scenario, we don't build protoc, so we need
    # to find and import it so downstream usage will use the host compiler.

    # find_program will search the host paths if CMAKE_FIND_ROOT_PATH_MODE_PROGRAM 
    # is configured correctly (see toolchain file)
    find_program(protoc_path NAMES protoc REQUIRED)
    add_executable(protoc IMPORTED)
    set_target_properties(protoc PROPERTIES IMPORTED_LOCATION ${protoc_path})
    set_target_properties(protoc PROPERTIES IMPORTED_GLOBAL TRUE)
    
    add_executable(protobuf::protoc ALIAS protoc)

    message(STATUS "protoc has been automatically imported from ${protoc_path} for crosscompiling.")

  elseif(NOT TARGET protobuf::protoc)
    add_executable(protobuf::protoc ALIAS protoc)
  endif()
    
  get_target_property(is_imported protoc IMPORTED)
  if (NOT is_imported)
    # Disable warnings for third-party lib
    target_compile_options(protoc PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/W0>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
    )
  endif()

  set(Protobuf_PROTOC_EXECUTABLE protoc)
  set(Protobuf_VERSION 3.11.4)
endif()

if (ECAL_THIRDPARTY_BUILD_SPDLOG)
  list(APPEND as_subproject spdlog)
  set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "My option" FORCE)
  set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "My option" FORCE)
  set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "My option" FORCE)
  add_subdirectory(thirdparty/spdlog)

  if (NOT TARGET spdlog::spdlog)
    add_library(spdlog::spdlog ALIAS spdlog)
    get_target_property(is_imported spdlog IMPORTED)
    if (NOT is_imported)
      # Disable warnings for third-party lib
      target_compile_options(spdlog PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W0 /wd4996>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
      )
    endif()
  endif()

endif()

if (ECAL_THIRDPARTY_BUILD_TINYXML2)
  list(APPEND as_subproject tinyxml2)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "My option" FORCE)
  set(DBUILD_STATIC_LIBS ON CACHE BOOL "My option" FORCE)
  set(BUILD_TESTING OFF CACHE BOOL "My option" FORCE)
  set(BUILD_TESTS OFF CACHE BOOL "My option" FORCE)
  add_subdirectory(thirdparty/tinyxml2 EXCLUDE_FROM_ALL)

  if (NOT TARGET tinyxml2::tinyxml2)
    add_library(tinyxml2::tinyxml2 ALIAS tinyxml2)
    get_target_property(is_imported tinyxml2 IMPORTED)
    if (NOT is_imported)
      # Disable warnings for third-party lib
      target_compile_options(tinyxml2 PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
      )
    endif ()
  endif ()

endif ()

if (ECAL_THIRDPARTY_BUILD_FINEFTP)
  list(APPEND as_subproject fineftp)
  add_subdirectory(thirdparty/fineftp-server/fineftp-server EXCLUDE_FROM_ALL)
  set_property(TARGET server PROPERTY FOLDER lib/fineftp)
  add_library(fineftp::server ALIAS server)
endif ()

if (ECAL_THIRDPARTY_BUILD_TERMCOLOR)
  list(APPEND as_subproject termcolor)
  add_subdirectory(thirdparty/termcolor EXCLUDE_FROM_ALL)
endif ()

# if (ECAL_THIRDPARTY_BUILD_LIBSSH2)
  # list(APPEND as_subproject libssh2)
  # set(DBUILD_STATIC_LIBS OFF CACHE BOOL "My option" FORCE)
  # set(BUILD_TESTING OFF CACHE BOOL "My option" FORCE)
  # add_subdirectory(thirdparty/libssh2)
  # add_library(LibSSH2 ALIAS libssh2)
# endif ()

# if (ECAL_THIRDPARTY_BUILD_ZLIB)
  # list(APPEND as_subproject zlib zlibstatic)
  # add_subdirectory(thirdparty/zlib)
  # add_library(ZLIB::zlibstatic ALIAS zlibstatic)
  # add_library(ZLIB::zlib ALIAS zlib)
# endif ()

if (ECAL_THIRDPARTY_BUILD_CURL)
  list(APPEND as_subproject CURL)
  
  if(WIN32)
    set(CMAKE_USE_WINSSL ON CACHE BOOL "Use native SSL on Windows" FORCE)
  endif()
  
  set(BUILD_CURL_EXE OFF CACHE BOOL "Don't build the curl executable" FORCE)
  set(ENABLE_MANUAL  OFF CACHE BOOL "Disable built-in manual" FORCE)
  
  add_subdirectory(thirdparty/curl)
  
  if (NOT TARGET CURL::libcurl)
    add_library(CURL::libcurl ALIAS libcurl)
    get_target_property(is_imported libcurl IMPORTED)
    if (NOT is_imported)
      # Disable Warnings
      target_compile_options(libcurl PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
      )
    endif()
  endif()
endif()

if (ECAL_THIRDPARTY_BUILD_GTEST)
  list(APPEND as_subproject GTest)
  # Googletest automatically forces MT instead of MD if we do not set this option.
  if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "My option" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "My option" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "My option" FORCE)
  endif()
  add_subdirectory(thirdparty/googletest EXCLUDE_FROM_ALL)
  if(NOT TARGET GTest::GTest)
    add_library(GTest::GTest ALIAS gtest)
  endif()
  if(NOT TARGET GTest::Main)
    add_library(GTest::Main ALIAS gtest_main)
  endif()
endif()

if (ECAL_THIRDPARTY_BUILD_HDF5)
  include(thirdparty/CPM/CMAKE/CPM.cmake)
  CPMAddPackage(
    NAME              hdf5
    #GIT_REPOSITORY    https://github.com/HDFGroup/hdf5.git
    #GIT_TAG           hdf5-1_10_7
    SOURCE_DIR        "${ECAL_PROJECT_ROOT}/thirdparty/hdf5"
    OPTIONS
      "HDF5_GENERATE_HEADERS OFF"
      "ONLY_SHARED_LIBS ON"
      "HDF5_ENABLE_THREADSAFE ON"
      "BUILD_TESTING OFF"
      "HDF5_BUILD_TOOLS OFF"
      "HDF5_BUILD_EXAMPLES OFF"
      "HDF5_BUILD_CPP_LIB OFF"
      "HDF5_BUILD_HL_LIB OFF"
      "BUILD_SHARED_LIBS ON"
  )
  
  if(NOT TARGET hdf5::hdf5-shared)
    FetchContent_GetProperties(hdf5)
    add_library(hdf5::hdf5-shared ALIAS hdf5-shared)
    target_include_directories(hdf5-shared INTERFACE "$<BUILD_INTERFACE:${hdf5_BINARY_DIR}>")
    target_compile_options(hdf5-shared PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/W0>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
    )
  endif()
endif()

if (ECAL_THIRDPARTY_BUILD_PROTOBUF OR
    ECAL_THIRDPARTY_BUILD_SPDLOG OR
    ECAL_THIRDPARTY_BUILD_TINYXML2 OR
    ECAL_THIRDPARTY_BUILD_FINEFTP OR
    ECAL_THIRDPARTY_BUILD_ZLIB OR
    ECAL_THIRDPARTY_BUILD_LIBSSH2 OR
    ECAL_THIRDPARTY_BUILD_CURL OR
    ECAL_THIRDPARTY_BUILD_GTEST OR
    ECAL_THIRDPARTY_BUILD_HDF5
    )
  if(MSVC)
    message(STATUS "reset thirdparty warnings for windows platform ..")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_OLD}")
  endif()
endif()

if (ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS)
  list(APPEND as_subproject CMakeFunctions)
endif()

macro(find_package)
  if(NOT "${ARGV0}" IN_LIST as_subproject)
    _find_package(${ARGV})
  endif()
endmacro()

if (ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS)
  add_subdirectory(thirdparty/cmake_functions)
endif ()


find_package(CMakeFunctions REQUIRED)

git_revision_information(DEFAULT ${ECAL_BUILD_VERSION})
set(eCAL_VERSION_MAJOR  ${GIT_REVISION_MAJOR})
set(eCAL_VERSION_MINOR  ${GIT_REVISION_MINOR})
set(eCAL_VERSION_PATCH  ${GIT_REVISION_PATCH})
set(eCAL_VERSION_STRING ${eCAL_VERSION_MAJOR}.${eCAL_VERSION_MINOR}.${eCAL_VERSION_PATCH})
set(eCAL_VERSION        ${eCAL_VERSION_STRING})

include(helper_functions/ecal_add_functions)
include(helper_functions/ecal_helper_functions)
include(helper_functions/ecal_install_functions)
include(helper_functions/ecal_python_functions)

if(MSVC)
  set(eCAL_PLATFORM_TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET})
endif()

# --------------------------------------------------------
# detect qt library
# --------------------------------------------------------
if (HAS_QT5)
  find_package(Qt5 COMPONENTS Core QUIET)
  if (NOT "${Qt5_FOUND}")
    autodetect_qt5_msvc_dir()
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

set(eCAL_config_dir                ${eCAL_BINARY_DIR}/cmake/)
set(eCAL_config                    ${eCAL_config_dir}/eCALConfig.cmake)
set(eCAL_config_version            ${eCAL_config_dir}/eCALConfigVersion.cmake)
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
endif()

# --------------------------------------------------------
# qt applications
# --------------------------------------------------------
if(BUILD_APPS AND HAS_QT5)
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
if(BUILD_ECAL_TESTS)
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
  # TODO: check this for github actions (ubuntu) - send failed because of broken pipe
  #add_subdirectory(testing/ecal/clientserver_test)
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
