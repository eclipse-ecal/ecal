cmake_minimum_required(VERSION 3.0...4.0)

project(hello_world_rec)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(eCAL REQUIRED)

set(source_files
  main.cpp
)

add_executable(${PROJECT_NAME} ${source_files})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  eCAL::string_core
)
