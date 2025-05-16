cmake_minimum_required(VERSION 3.10)
project(network_crash)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

if(NOT DEFINED ECALCONFIGHELPER_PATH)
  set(ECALCONFIGHELPER_PATH ${CMAKE_CURRENT_LIST_DIR}/../../lib/EcalConfigHelper)
else()
  add_subdirectory(${ECALCONFIGHELPER_PATH} ${CMAKE_BINARY_DIR}/EcalConfigHelper)  
endif()

add_executable(network_crash_publisher network_crash_publisher.cpp)
add_executable(network_crash_subscriber network_crash_subscriber.cpp)

target_include_directories(network_crash_publisher
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(network_crash_subscriber
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(network_crash_publisher 
  PRIVATE  
    EcalConfigHelper 
    eCAL::core
)

target_link_libraries(network_crash_subscriber 
  PRIVATE    
    EcalConfigHelper
    eCAL::core
)