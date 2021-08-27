set(protobuf_BUILD_TESTS OFF CACHE BOOL "My option" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "My option" FORCE)
if(UNIX)
  set(protobuf_BUILD_SHARED_LIBS ON CACHE BOOL "My option" FORCE)
endif()
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

if (NOT TARGET protoc AND CMAKE_CROSSCOMPILING)
  # In cross compile scenario, we don't build protoc, so we need
  # to find and import it so downstream usage will use the host compiler.

  # find_program will search the host paths if CMAKE_FIND_ROOT_PATH_MODE_PROGRAM 
  # is configured correctly (see toolchain file)
  find_program(protoc_path NAMES protoc REQUIRED)
  add_executable(protoc IMPORTED)
  set_target_properties(protoc PROPERTIES IMPORTED_LOCATION ${protoc_path})
  set_target_properties(protoc PROPERTIES IMPORTED_GLOBAL TRUE)
  
  add_executable(protobuf::protoc ALIAS protoc)

  message(STATUS "protoc has been automatically imported from ${protoc_path} for crosscompiling.")

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

set(Protobuf_PROTOC_EXECUTABLE protoc)
set(Protobuf_VERSION 3.11.4)