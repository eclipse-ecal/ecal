cmake_minimum_required(VERSION 3.10)

project(pubsub_ecal_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# eCAL include and lib
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# Sources
set(SOURCES
    publisher.cpp
    subscriber.cpp
    ecal_config_helper.cpp
)

# Publisher
add_executable(publisher publisher.cpp ecal_config_helper.cpp)
target_link_libraries(publisher ecal_core ecal)

# Subscriber
add_executable(subscriber subscriber.cpp ecal_config_helper.cpp)
target_link_libraries(subscriber ecal_core ecal)
