cmake_minimum_required(VERSION 3.10)
project(sub_crash)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(eCAL REQUIRED)

if(NOT DEFINED ECALCONFIGHELPER_PATH)
  set(ECALCONFIGHELPER_PATH ${CMAKE_CURRENT_LIST_DIR}/../../lib/EcalConfigHelper)
else()
  add_subdirectory(${ECALCONFIGHELPER_PATH} ${CMAKE_BINARY_DIR}/EcalConfigHelper)  
endif()

add_executable(test_subscriber test_subscriber.cpp)
add_executable(test_publisher test_publisher.cpp)
add_executable(crash_subscriber crash_subscriber.cpp)
add_executable(monitoring monitoring.cpp)

target_include_directories(test_subscriber
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(test_publisher
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(crash_subscriber
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(monitoring
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)



target_link_libraries(test_subscriber 
  PRIVATE    
    EcalConfigHelper 
    eCAL::core
)

target_link_libraries(test_publisher
  PRIVATE
    EcalConfigHelper
    eCAL::core
)

target_link_libraries(crash_subscriber
  PRIVATE 
    EcalConfigHelper
    eCAL::core
)

target_link_libraries(monitoring
  PRIVATE 
    EcalConfigHelper
    eCAL::core
)