cmake_minimum_required(VERSION 3.10)
project(pub_sub_crash)

find_package(EcalConfigHelper REQUIRED)
find_package(MonitoringLibrary REQUIRED)
find_package(eCAL REQUIRED)

add_executable(crash_publisher crash_publisher.cpp)
add_executable(crash_subscriber crash_subscriber.cpp)

target_link_libraries(crash_publisher 
    EcalConfigHelper::EcalConfigHelper 
    MonitoringLibrary::MonitoringLibrary 
    eCAL::core
)

target_link_libraries(crash_subscriber 
    EcalConfigHelper::EcalConfigHelper 
    MonitoringLibrary::MonitoringLibrary 
    eCAL::core
)
