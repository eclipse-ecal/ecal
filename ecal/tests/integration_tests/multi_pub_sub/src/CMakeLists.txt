cmake_minimum_required(VERSION 3.10)
project(multi_pub_sub)

find_package(EcalConfigHelper REQUIRED)
find_package(eCAL REQUIRED)

add_executable(multi_publisher multi_publisher.cpp)
add_executable(multi_subscriber multi_subscriber.cpp)
add_executable(multi_publisher2 multi_publisher2.cpp)
add_executable(multi_subscriber2 multi_subscriber2.cpp)

target_link_libraries(multi_publisher 
    EcalConfigHelper::EcalConfigHelper 
    eCAL::core
)

target_link_libraries(multi_subscriber
    EcalConfigHelper::EcalConfigHelper 
    eCAL::core
)

target_link_libraries(multi_publisher2 
    EcalConfigHelper::EcalConfigHelper 
    eCAL::core
)

target_link_libraries(multi_subscriber2 
    EcalConfigHelper::EcalConfigHelper 
    eCAL::core
)