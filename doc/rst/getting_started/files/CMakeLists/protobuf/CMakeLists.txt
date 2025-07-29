cmake_minimum_required(VERSION 3.16)

# This CMakeLists.txt is used to build a protobuf example for eCAL.
# So we name this project, which is also the name of the executable, accordingly.
project(ecal_hello_world_send_protobuf)

# We set the C++ standard to C++14, which is currently required by eCAL.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to find the eCAL installation and the protobuf library.
find_package(eCAL REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# Set a list all source files for the project.
# In our case it is only one file, but can be simply extended to multiple files.
set(source_files
  hello_world_send_protobuf.cpp
)

# We set a list of protobuf files here. The path is relative to the CMakeLists.txt file.
# This CMakeLists.txt assumes that the protobuf files are located in the ../proto_messages/ directory.
set(protobuf_files
    ${CMAKE_CURRENT_SOURCE_DIR}/../proto_messages/hello_world.proto
)

# Now we add the source file to the executable to be generated.
# The name of the executable is the same as the project name.
add_executable(${PROJECT_NAME} ${source_files})

# We need to tell CMake to generate the protobuf files. This is done by the PROTOBUF_TARGET_CPP function.
# This compiles the .proto file to a C++ header, in our case "hello_world.pb.h".
# Note that the PROTOBUF_TARGET_CPP function is a eCAL convenience function. 
# You can also use the protobuf_generate_cpp function from the protobuf package directly if you are already familiar with it.
PROTOBUF_TARGET_CPP(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/../proto_messages/ ${protobuf_files})

# Finally we need to link the necessary libararies to the executable.
# Which target you link to depends on the type of the message you are using.
# In our case we use the protobuf message, so we link to the eCAL protobuf core library.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  eCAL::protobuf_core
)
