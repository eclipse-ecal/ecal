# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
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

# -----------------------------------------------------------------------------
# Function: PROTOBUF_GENERATE_CSHARP
# Description: Generates C# files from .proto files and places them in folders
#              matching the proto folder structure under ${PROTO_OUT_DIR}.
# -----------------------------------------------------------------------------
function(PROTOBUF_GENERATE_CSHARP SRCS_RET PROTO_OUT_DIR_RET PROTO_ROOT)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CSHARP() called without any proto files")
    return()
  endif()

  # Ensure protoc target exists
  if (NOT TARGET protobuf::protoc)
    if (Protobuf_PROTOC_EXECUTABLE)
      add_executable(protobuf::protoc IMPORTED)
      set_target_properties(protobuf::protoc PROPERTIES
        IMPORTED_LOCATION "${Protobuf_PROTOC_EXECUTABLE}"
      )
    else()
      message(FATAL_ERROR "Neither protobuf::protoc nor Protobuf_PROTOC_EXECUTABLE is defined.")
    endif()
  endif()

  set(PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/protobuf")
  set(FLAT_OUT_DIR "${PROTO_OUT_DIR}/_flat")
  file(MAKE_DIRECTORY "${FLAT_OUT_DIR}")
  get_filename_component(PROTO_ROOT "${PROTO_ROOT}" ABSOLUTE)

  set(SRCS "")

  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL "${FIL}" ABSOLUTE)
    file(RELATIVE_PATH REL_FIL "${PROTO_ROOT}" "${ABS_FIL}")
    get_filename_component(PROTO_DIR "${REL_FIL}" DIRECTORY)
    get_filename_component(FILENAME_WE "${REL_FIL}" NAME_WE)

    set(FLAT_CS_FILE "${FLAT_OUT_DIR}/${FILENAME_WE}.cs")
    set(FINAL_CS_FILE "${PROTO_OUT_DIR}/${PROTO_DIR}/${FILENAME_WE}.cs")

    list(APPEND SRCS "${FINAL_CS_FILE}")
    file(MAKE_DIRECTORY "${PROTO_OUT_DIR}/${PROTO_DIR}")

    add_custom_command(
      OUTPUT "${FINAL_CS_FILE}"
      COMMAND protobuf::protoc
      ARGS "--proto_path=." "--csharp_out=${FLAT_OUT_DIR}" "${REL_FIL}"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FLAT_CS_FILE}" "${FINAL_CS_FILE}"
      WORKING_DIRECTORY "${PROTO_ROOT}"
      DEPENDS "${ABS_FIL}" protobuf::protoc
      COMMENT "Generating and relocating CSharp file for ${REL_FIL}"
      VERBATIM
    )
  endforeach()

  set_source_files_properties(${SRCS} PROPERTIES GENERATED TRUE)
  set(${SRCS_RET} "${SRCS}" PARENT_SCOPE)
  set(${PROTO_OUT_DIR_RET} "${PROTO_OUT_DIR}" PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# Function: PROTOBUF_ADD_CS_TO_TARGET
# Description: Adds the .cs files to the CMake target and organizes them
#              into source groups matching folder structure (for IDEs).
# -----------------------------------------------------------------------------
function(PROTOBUF_ADD_CS_TO_TARGET TARGET_NAME)
  set(multiValueArgs PROTO_FILES CS_FILES)

  cmake_parse_arguments(INPUT "" "" "${multiValueArgs}" ${ARGN})

  target_sources(${TARGET_NAME} PRIVATE ${INPUT_CS_FILES})

  foreach(CS_FILE ${INPUT_CS_FILES})
    file(RELATIVE_PATH REL_CS_PATH "${CMAKE_CURRENT_BINARY_DIR}/protobuf" "${CS_FILE}")
    get_filename_component(SOURCE_GROUP_PATH "${REL_CS_PATH}" DIRECTORY)
    string(REPLACE "/" "\\" SOURCE_GROUP_PATH "${SOURCE_GROUP_PATH}")
    source_group("protobuf\\cs\\${SOURCE_GROUP_PATH}" FILES ${CS_FILE})
  endforeach()
endfunction()

# -----------------------------------------------------------------------------
# Function: PROTOBUF_TARGET_CSHARP
# Description: Orchestrates generation, grouping, and optional install of C# sources.
# -----------------------------------------------------------------------------
function(PROTOBUF_TARGET_CSHARP TARGET_NAME PROTO_ROOT)
  set(oneValueArgs INSTALL_FOLDER)
  cmake_parse_arguments(PROTOBUF_TARGET_CSHARP "" "${oneValueArgs}" "" ${ARGN})

  PROTOBUF_GENERATE_CSHARP(proto_csharp proto_out_dir ${PROTO_ROOT} ${PROTOBUF_TARGET_CSHARP_UNPARSED_ARGUMENTS})

  PROTOBUF_ADD_CS_TO_TARGET(${TARGET_NAME}
    PROTO_FILES ${PROTOBUF_TARGET_CSHARP_UNPARSED_ARGUMENTS}
    CS_FILES    ${proto_csharp}
  )

  if(PROTOBUF_TARGET_CSHARP_INSTALL_FOLDER)
    install(
      DIRECTORY "${proto_out_dir}/"
      DESTINATION "${PROTOBUF_TARGET_CSHARP_INSTALL_FOLDER}"
      FILES_MATCHING PATTERN "*.cs"
    )
  endif()
endfunction()
