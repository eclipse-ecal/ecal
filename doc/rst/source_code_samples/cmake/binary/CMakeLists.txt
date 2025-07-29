cmake_minimum_required(VERSION 3.16)
project(blob_send)

# Tell CMake to find the eCAL installation.
find_package(eCAL REQUIRED)

# We create an executable and add a source file.
add_executable(blob_send)
target_sources(blob_send
  PRIVATE
    blob_send.cpp
)

# Finally we need to link the necessary eCAL libraries to the executable.
# The appropriate target name can be taken from the target name table.
target_link_libraries(blob_send
  PRIVATE
    eCAL::core
)
