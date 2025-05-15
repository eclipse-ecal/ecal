cmake_minimum_required(VERSION 3.10)
project(network_crash)

find_package(EcalConfigHelper REQUIRED)
find_package(eCAL REQUIRED)

add_executable(network_crash_publisher network_crash_publisher.cpp)
add_executable(network_crash_subscriber network_crash_subscriber.cpp)

target_link_libraries(network_crash_publisher 
    EcalConfigHelper::EcalConfigHelper 
    eCAL::core
)

target_link_libraries(network_crash_subscriber 
    EcalConfigHelper::EcalConfigHelper 
    eCAL::core
)
