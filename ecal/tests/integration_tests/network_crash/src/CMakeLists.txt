cmake_minimum_required(VERSION 3.10)
project(network_crash)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(eCAL REQUIRED)

if(NOT DEFINED ECALCONFIGHELPER_PATH)
  set(ECALCONFIGHELPER_PATH ${CMAKE_CURRENT_LIST_DIR}/../../lib/EcalConfigHelper)
else()
  add_subdirectory(${ECALCONFIGHELPER_PATH} ${CMAKE_BINARY_DIR}/EcalConfigHelper)  
endif()

add_executable(local_udp_pub local_udp_pub.cpp)
add_executable(network_udp_pub network_udp_pub.cpp)
add_executable(network_crash_sub network_crash_sub.cpp)
add_executable(local_udp_pub2 local_udp_pub2.cpp)


target_include_directories(local_udp_pub PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(network_udp_pub PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(network_crash_sub PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(local_udp_pub2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(local_udp_pub PRIVATE EcalConfigHelper eCAL::core)
target_link_libraries(network_udp_pub PRIVATE EcalConfigHelper eCAL::core)
target_link_libraries(network_crash_sub PRIVATE EcalConfigHelper eCAL::core)
target_link_libraries(local_udp_pub2 PRIVATE EcalConfigHelper eCAL::core)