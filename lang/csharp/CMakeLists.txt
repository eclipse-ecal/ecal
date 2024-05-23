# C# build, can be executed standalone against an eCAL installation
project(ecal_csharp)

set(CMAKE_DOTNET_SDK "")
set(ECAL_CSHARP_PROTOBUF_VERSION "3.26.1" CACHE STRING "Protobuf Version for C# build")

option(ECAL_CSHARP_BUILD_SAMPLES "BUILD eCAL C# samples" ON)
option(ECAL_CSHARP_BUILD_TESTS   "BUILD eCAL C# unittests" ON)

add_subdirectory(Continental.eCAL.Core)
add_subdirectory(Continental.eCAL.Protobuf)
if (ECAL_CSHARP_BUILD_TESTS)
add_subdirectory(Continental.eCAL.Core.Test)
add_subdirectory(Continental.eCAL.Protobuf.Test)
endif()

if (ECAL_CSHARP_BUILD_SAMPLES)
add_subdirectory(Continental.eCAL.Core.Samples)
add_subdirectory(Continental.eCAL.Protobuf.Samples)
endif ()

if (ECAL_CSHARP_BUILD_SAMPLES OR ECAL_CSHARP_BUILD_TESTS)
#Contains the person.proto which are being compiled to cs files
add_subdirectory(Continental.eCAL.Protobuf.Person)
endif()
