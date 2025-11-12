# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2025 Continental Corporation
# Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================

project(core_pb)

find_package(Protobuf REQUIRED)
find_package(protozero REQUIRED)

add_library(core_protos INTERFACE)
add_library(eCAL::core_protos ALIAS core_protos)


function(PROTOBUF_GENERATE_PBFTAGS HDRS_RET PROTO_OUT_DIR_RET PROTO_ROOT COPY_DIRECTORY)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_PBFTAGS() called without any proto files")
    return()
  endif()
  
  #Backwards compatability
  if (NOT TARGET protobuf::protoc)
    if (Protobuf_PROTOC_EXECUTABLE)
      ADD_EXECUTABLE(protobuf::protoc IMPORTED)
      SET_TARGET_PROPERTIES(protobuf::protoc PROPERTIES
        IMPORTED_LOCATION "${Protobuf_PROTOC_EXECUTABLE}"
      )
    else ()
      message(FATAL_ERROR "Neither protobuf::protoc not the $(Protobuf_PROTOC_EXECUTABLE) variable is defined. Cannot generate protobuf files.")
    endif ()     	
  endif ()

  set(PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/protobuf")
  file(MAKE_DIRECTORY ${PROTO_OUT_DIR})  
  get_filename_component(PROTO_ROOT ${PROTO_ROOT} ABSOLUTE)

  foreach(FIL ${ARGN})
    # Make the file path absolute, so that it is also normalized (no a\..\a\b)
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    # Compute the relative path of the .proto file compared to the proto root
    FILE(RELATIVE_PATH REL_FIL ${PROTO_ROOT} ${FIL})
    # Remove the file extension
    string(REGEX REPLACE "\\.[^.]*$" "" REL_FIL_WE ${REL_FIL})

    set(ABSOLUTE_GENERATED_FILENAME "${PROTO_OUT_DIR}/${REL_FIL_WE}.pbftags.h")
    set(ABSOLUTE_GENERATED_FILENAME_CORE_COPY "${COPY_DIRECTORY}/${REL_FIL_WE}.pbftags.h")

    list(APPEND HDRS ${ABSOLUTE_GENERATED_FILENAME})
    list(APPEND COPIED_HDRS "${ABSOLUTE_GENERATED_FILENAME_CORE_COPY}")

    add_custom_command(
      OUTPUT "${ABSOLUTE_GENERATED_FILENAME}"
      COMMAND protobuf::protoc
      ARGS "--proto_path=${PROTO_ROOT}" "--plugin=protoc-gen-pbftags=$<TARGET_FILE:protoc-gen-pbftags>" "--pbftags_out=${PROTO_OUT_DIR}" ${ABS_FIL}
      DEPENDS ${ABS_FIL} protobuf::protoc protoc-gen-pbftags
      COMMENT "Running C++ protocol buffer compiler for pbftags on ${ABS_FIL}"
      VERBATIM )

    add_custom_command(
      OUTPUT ${ABSOLUTE_GENERATED_FILENAME_CORE_COPY}
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${ABSOLUTE_GENERATED_FILENAME}" "${ABSOLUTE_GENERATED_FILENAME_CORE_COPY}"
      DEPENDS "${ABSOLUTE_GENERATED_FILENAME}"
      COMMENT "Copy (if different) ${ABSOLUTE_GENERATED_FILENAME} -> ${ABSOLUTE_GENERATED_FILENAME_CORE_COPY}"
      VERBATIM
    )

  endforeach()

  set_source_files_properties(${HDRS} PROPERTIES GENERATED TRUE)
  set_source_files_properties(${COPIED_HDRS} PROPERTIES GENERATED TRUE)

  add_custom_target(core_pbftags_copies DEPENDS ${COPIED_HDRS})
  
  set(${HDRS_RET}          ${HDRS}          PARENT_SCOPE)
  set(${PROTO_OUT_DIR_RET} ${PROTO_OUT_DIR} PARENT_SCOPE)
endfunction()

function(PROTOBUF_TARGET_PBFTAGS)
  set(oneValueArgs    TARGET PROTO_ROOT VISIBILITY)
  set(multiValueArgs  PROTO_FILES)
  cmake_parse_arguments(PROTOBUF_TARGET_PBFTAGS 
    "" 
    "${oneValueArgs}"
    "${multiValueArgs}" ${ARGN} )
   
  # PROTOBUF_TARGET_CPP_UNPARSED_ARGUMENTS: These are all .proto files
  # INPUT_INSTALL_DIR: If this option is given, we want also to install files and and add them to the interface include path.
  
  PROTOBUF_GENERATE_PBFTAGS(
    proto_headers
    proto_include_dirs 
    ${PROTOBUF_TARGET_PBFTAGS_PROTO_ROOT}
    ${ECAL_CORE_PROJECT_ROOT}/core/src/serialization/protozero
    ${PROTOBUF_TARGET_PBFTAGS_PROTO_FILES} 
  )

  target_sources(${PROTOBUF_TARGET_PBFTAGS_TARGET}
    ${PROTOBUF_TARGET_PBFTAGS_VISIBILITY}    
    FILE_SET pbftags
    TYPE HEADERS
    BASE_DIRS ${proto_include_dirs}
    FILES 
      ${proto_headers}
)
endfunction()



set(proto_root
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set(proto_files
  ${proto_root}/ecal/core/pb/datatype.proto
  ${proto_root}/ecal/core/pb/ecal.proto
  ${proto_root}/ecal/core/pb/host.proto
  ${proto_root}/ecal/core/pb/layer.proto
  ${proto_root}/ecal/core/pb/logging.proto
  ${proto_root}/ecal/core/pb/monitoring.proto
  ${proto_root}/ecal/core/pb/process.proto
  ${proto_root}/ecal/core/pb/service.proto
  ${proto_root}/ecal/core/pb/topic.proto
)

target_sources(core_protos
  PUBLIC    
    FILE_SET core_protos
    TYPE HEADERS
    BASE_DIRS ${proto_root}
    FILES 
      ${proto_files}
)

# Compile statically on Windows and shared for all other systems. 
# 
# We compile shared on Linux etc., as we must only load the proto descriptors 
# once; otherwise, protobuf would throw an exception on runtime. A shared object
# achieves that, as it is only loaded into memory once, even when being linked 
# against from different libraries.
# We don't have to do that on Windows, as we compile against protobuf statically 
# and therefore each .dll has it's own descriptor pool. Having a static lib here 
# also has the advantage that we need to export the symbols, which is default 
# disabled on Windows.
# 
# TODO: Having code like that probably isn't the best solution ever. We should 
# maybe always link against protobuf statically. Currently the reason why we 
# don't do that is, that the default Ubuntu libprotobuf.a doesn't contain 
# position independent code and can therefore not be statically compiled into a 
# shared object library.
if (WIN32)
  ecal_add_static_library(${PROJECT_NAME} src/core_pb.cpp)
else()
  ecal_add_shared_library(${PROJECT_NAME} src/core_pb.cpp)
endif()

add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

protobuf_target_cpp(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src
  INSTALL_FOLDER "${eCAL_install_include_dir}"
  ${proto_files}
)

target_compile_options(${PROJECT_NAME}
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4505 /wd4592 /wd4189>
    $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-parameter>)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

ecal_install_library(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER core)

if (ECAL_CORE_GENERATE_PBFTAGS)
add_library(core_pbftags INTERFACE)
add_library(eCAL::core_pbftags ALIAS core_pbftags)

PROTOBUF_TARGET_PBFTAGS( 
  TARGET core_pbftags
  PROTO_ROOT ${proto_root}
  VISIBILITY INTERFACE
  PROTO_FILES ${proto_files}
)

target_link_libraries(core_pbftags INTERFACE protozero::protozero)

# eCAL core needs to depend on this target, if available. 
# In SDK builds only, we rely on checked in files instead
add_dependencies(ecal_core_serialization core_pbftags_copies)
endif()