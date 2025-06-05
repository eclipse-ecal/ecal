cmake_minimum_required(VERSION 3.10)
project(basic_pub_sub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

if(NOT DEFINED ECALCONFIGHELPER_PATH)
  set(ECALCONFIGHELPER_PATH ${CMAKE_CURRENT_LIST_DIR}/../../lib/EcalConfigHelper)
else()
  add_subdirectory(${ECALCONFIGHELPER_PATH} ${CMAKE_BINARY_DIR}/EcalConfigHelper)  
endif()

add_executable(one_publisher one_publisher.cpp)
add_executable(one_subscriber one_subscriber.cpp)

target_include_directories(one_publisher
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(one_subscriber
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link Libraries
target_link_libraries(one_publisher 
  PRIVATE
    eCAL::core
    EcalConfigHelper
)

target_link_libraries(one_subscriber 
  PRIVATE
    eCAL::core
    EcalConfigHelper
)
