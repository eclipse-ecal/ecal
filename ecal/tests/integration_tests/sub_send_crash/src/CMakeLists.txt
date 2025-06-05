cmake_minimum_required(VERSION 3.10)
project(sub_send_crash)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(eCAL REQUIRED)

# EcalConfigHelper library path (can be overridden)
if(NOT DEFINED ECALCONFIGHELPER_PATH)
  set(ECALCONFIGHELPER_PATH ${CMAKE_CURRENT_LIST_DIR}/../../lib/EcalConfigHelper)
else()
  add_subdirectory(${ECALCONFIGHELPER_PATH} ${CMAKE_BINARY_DIR}/EcalConfigHelper)
endif()

# Executables
add_executable(test_subscriber test_subscriber.cpp)
add_executable(large_publisher large_publisher.cpp)
add_executable(zero_copy_pub zero_copy_pub.cpp)
add_executable(crash_send_subscriber crash_send_subscriber.cpp)

# Include directories
target_include_directories(test_subscriber
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(large_publisher
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(zero_copy_pub
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(crash_send_subscriber
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Linking
target_link_libraries(test_subscriber
  PRIVATE
    EcalConfigHelper
    eCAL::core
)

target_link_libraries(large_publisher
  PRIVATE
    EcalConfigHelper
    eCAL::core
)

target_link_libraries(zero_copy_pub
  PRIVATE
    EcalConfigHelper
    eCAL::core
)

target_link_libraries(crash_send_subscriber
  PRIVATE
    EcalConfigHelper
    eCAL::core
)
