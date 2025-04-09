# Protobuf 4(.25.X) is skipped because of a regression with msvc that never
# had its fix backported.
# https://github.com/protocolbuffers/protobuf/issues/14602
set(Protobuf_PROTOC_EXECUTABLE protoc)
set(Protobuf_VERSION 5.29.4)
set(Protobuf_VERSION_MAJOR 5)
set(Protobuf_VERSION_MINOR 29)
set(Protobuf_VERSION_PATCH 4)

include_guard(GLOBAL)

set(protobuf_BUILD_TESTS OFF CACHE BOOL "My option" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "My option" FORCE)
if(UNIX)
  set(protobuf_BUILD_SHARED_LIBS ON CACHE BOOL "My option" FORCE)
endif()

set(protobuf_ABSL_PROVIDER "package")

if(MSVC)
  message(STATUS "supress thirdparty warnings for windows platform ..")
  set(CMAKE_CXX_FLAGS_OLD "${CMAKE_CXX_FLAGS}")
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W0" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
  endif()
endif()

ecal_disable_all_warnings()
ecal_variable_push(CMAKE_POLICY_VERSION_MINIMUM)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/protobuf" "${eCAL_BINARY_DIR}/thirdparty/protobuf" SYSTEM)
ecal_variable_pop(CMAKE_POLICY_VERSION_MINIMUM)
ecal_restore_warning_level()

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

if (TARGET libprotobuf)
set_property(TARGET libprotobuf PROPERTY FOLDER thirdparty/protobuf)
endif ()

if (TARGET libprotobuf-lite)
set_property(TARGET libprotobuf-lite PROPERTY FOLDER thirdparty/protobuf)
endif () 

if (TARGET libprotoc)
set_property(TARGET libprotoc PROPERTY FOLDER thirdparty/protobuf)
endif ()

if (TARGET protoc)
set_property(TARGET protoc PROPERTY FOLDER thirdparty/protobuf)
endif ()
