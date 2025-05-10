cmake_minimum_required(VERSION 3.10)
project(basic_pub_sub)

find_package(EcalConfigHelper REQUIRED)
find_package(MonitoringLibrary REQUIRED)
find_package(eCAL REQUIRED)

add_executable(one_publisher one_publisher.cpp)
add_executable(one_subscriber one_subscriber.cpp)

target_link_libraries(one_publisher 
    EcalConfigHelper::EcalConfigHelper 
    MonitoringLibrary::MonitoringLibrary 
    eCAL::core
)

target_link_libraries(one_subscriber 
    EcalConfigHelper::EcalConfigHelper 
    MonitoringLibrary::MonitoringLibrary 
    eCAL::core
)
