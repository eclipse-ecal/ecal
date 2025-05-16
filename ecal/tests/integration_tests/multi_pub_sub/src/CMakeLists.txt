cmake_minimum_required(VERSION 3.10)
project(multi_pub_sub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

if(NOT DEFINED ECALCONFIGHELPER_PATH)
  set(ECALCONFIGHELPER_PATH ${CMAKE_CURRENT_LIST_DIR}/../../lib/EcalConfigHelper)
else()
  add_subdirectory(${ECALCONFIGHELPER_PATH} ${CMAKE_BINARY_DIR}/EcalConfigHelper)  
endif()

add_executable(multi_publisher multi_publisher.cpp)
add_executable(multi_subscriber multi_subscriber.cpp)
add_executable(multi_publisher2 multi_publisher2.cpp)
add_executable(multi_subscriber2 multi_subscriber2.cpp)

target_include_directories(multi_publisher
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(multi_publisher2
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(multi_subscriber
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(multi_subscriber2
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(multi_publisher 
  PRIVATE    
    EcalConfigHelper 
    eCAL::core
)

target_link_libraries(multi_subscriber
  PRIVATE
    EcalConfigHelper
    eCAL::core
)

target_link_libraries(multi_publisher2
  PRIVATE 
    EcalConfigHelper
    eCAL::core
)

target_link_libraries(multi_subscriber2
  PRIVATE 
    EcalConfigHelper
    eCAL::core
)
