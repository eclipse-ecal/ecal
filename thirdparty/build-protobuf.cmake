set(protobuf_BUILD_TESTS OFF CACHE BOOL "My option" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "My option" FORCE)
if(UNIX)
  set(protobuf_BUILD_SHARED_LIBS ON CACHE BOOL "My option" FORCE)
endif()
set(Protobuf_PROTOC_EXECUTABLE "" CACHE FILEPATH "Path to protoc executable")

add_subdirectory(thirdparty/protobuf/cmake)

if (NOT TARGET protobuf::libprotobuf)
  add_library(protobuf::libprotobuf ALIAS libprotobuf)
endif()

get_target_property(is_imported libprotobuf IMPORTED)
if (NOT is_imported)
  # Disable warnings for third-party lib
  target_compile_options(libprotobuf PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
  )
endif()

if (CMAKE_CROSSCOMPILING)
  # In cross compile scenario, we don't build protoc, so we need
  # to find and import it so downstream usage will use the host compiler.

  # find_program will search the host paths if CMAKE_FIND_ROOT_PATH_MODE_PROGRAM 
  # is configured correctly (see toolchain file)
  if (NOT Protobuf_PROTOC_EXECUTABLE)
    find_program(Protobuf_PROTOC_EXECUTABLE NAMES protoc_host REQUIRED)
  endif ()
  add_executable(protoc_host IMPORTED)
  set_target_properties(protoc_host PROPERTIES IMPORTED_LOCATION ${Protobuf_PROTOC_EXECUTABLE})
  set_target_properties(protoc_host PROPERTIES IMPORTED_GLOBAL TRUE)
  
  add_executable(protobuf::protoc ALIAS protoc_host)

  message(STATUS "protoc has been set to ${Protobuf_PROTOC_EXECUTABLE} for crosscompiling.")

elseif(NOT TARGET protobuf::protoc)
  add_executable(protobuf::protoc ALIAS protoc)
endif()
  
get_target_property(is_imported protoc IMPORTED)
if (NOT is_imported)
  # Disable warnings for third-party lib
  target_compile_options(protoc PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
  )
endif()

set(Protobuf_VERSION 3.11.4)
