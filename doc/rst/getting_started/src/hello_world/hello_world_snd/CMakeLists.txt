cmake_minimum_required(VERSION 3.0)

project(hello_world_snd)

find_package(eCAL REQUIRED)

set(source_files
  main.cpp
)

add_executable(${PROJECT_NAME} ${source_files})

target_link_libraries(${PROJECT_NAME}
  eCAL::core
)
