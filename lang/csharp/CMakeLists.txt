# C# build, can be executed standalone against an eCAL installation
project(ecal_csharp)

set(CMAKE_DOTNET_SDK "")
set(ECAL_CSHARP_PROTOBUF_VERSION "3.11.4" CACHE STRING "Protobuf Version for C# build")
option(ECAL_CSHARP_BUILD_SAMPLES "BUILD eCAL C# samples" ON)

add_subdirectory(Continental/eCAL/Core)
add_subdirectory(Continental/eCAL/Protobuf)

if (ECAL_CSHARP_BUILD_SAMPLES)
add_subdirectory(../../samples/csharp samples)
endif ()