cmake_minimum_required(VERSION 3.10)
project(sub_crash)

find_package(EcalConfigHelper REQUIRED)
find_package(eCAL REQUIRED)

add_executable(test_subscriber test_subscriber.cpp)
add_executable(test_publisher test_publisher.cpp)
add_executable(crash_subscriber crash_subscriber.cpp)
add_executable(monitoring monitoring.cpp)

target_link_libraries(test_subscriber
  EcalConfigHelper::EcalConfigHelper
  eCAL::core
)

target_link_libraries(test_publisher
  EcalConfigHelper::EcalConfigHelper
  eCAL::core
)

target_link_libraries(crash_subscriber
  EcalConfigHelper::EcalConfigHelper
  eCAL::core
)

target_link_libraries(monitoring
  EcalConfigHelper::EcalConfigHelper
  eCAL::core
)
