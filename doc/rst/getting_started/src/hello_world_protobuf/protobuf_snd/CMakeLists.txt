cmake_minimum_required(VERSION 3.0)

project(protobuf_snd)

if(MSVC)
  # CMake >= 3.15 will erroneously define PROTOBUF_USE_DLLS otherwise
  set (Protobuf_USE_STATIC_LIBS TRUE) 
endif()

find_package(eCAL REQUIRED)
find_package(Protobuf REQUIRED)

set(source_files
  main.cpp
)

set(protobuf_files
    ${CMAKE_CURRENT_SOURCE_DIR}/../proto_messages/hello_world.proto
)

add_executable(${PROJECT_NAME} ${source_files})

PROTOBUF_TARGET_CPP(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/../proto_messages/ ${protobuf_files})

target_link_libraries(${PROJECT_NAME}
  eCAL::core
  protobuf::libprotobuf
)
