cmake_minimum_required(VERSION 3.5.1)

# Normalize backslashes from Windows paths
file(TO_CMAKE_PATH "${CMAKE_MODULE_PATH}" CMAKE_MODULE_PATH)
file(TO_CMAKE_PATH "${CMAKE_PREFIX_PATH}" CMAKE_PREFIX_PATH)
message(STATUS "Module Path: ${CMAKE_MODULE_PATH}")
message(STATUS "Prefix Path: ${CMAKE_PREFIX_PATH}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(tcpub)

set(as_subproject tcpub)
include(sourcetree.cmake)

string(COMPARE EQUAL "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" _cmp)
if (_cmp)
  macro(find_package)
    if(NOT "${ARGV0}" IN_LIST as_subproject)
      _find_package(${ARGV})
    endif()
  endmacro()
endif()

add_subdirectory(tcpub)

add_subdirectory(thirdparty/recycle EXCLUDE_FROM_ALL)

add_subdirectory(performance_publisher)
add_subdirectory(performance_subscriber)
add_subdirectory(hello_world_publisher)
add_subdirectory(hello_world_subscriber)

add_subdirectory(ecal_to_tcp)
add_subdirectory(tcp_to_ecal)
