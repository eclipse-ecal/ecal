cmake_minimum_required(VERSION 3.16)

# This CMakeLists.txt is used to build a protobuf example for eCAL.
# So we name this project, which is also the name of the executable, accordingly.
project(hello_world_send)

# We set the C++ standard to C++14, which is currently required by eCAL.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to find the eCAL installation.
find_package(eCAL REQUIRED)

# Set a list all source files for the project.
# In our case it is only one file, but can be simply extended to multiple files.
set(source_files
  hello_world_send.cpp
)

# Now we add the source file to the executable to be generated.
# The name of the executable is the same as the project name.
add_executable(${PROJECT_NAME} ${source_files})

# Finally we need to link the necessary libararies to the executable.
# Which target you link to depends on the type of the message you are using.
# In our case we use the string message, so we link to the eCAL string core library.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  eCAL::string_core
)
