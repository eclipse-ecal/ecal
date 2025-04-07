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

function(PROTOBUF_GENERATE_CSHARP SRCS_RET PROTO_OUT_DIR_RET PROTO_ROOT)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP_EXT() called without any proto files")
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

  message(STATUS ${Protobuf_PROTOC_EXECUTABLE})

  foreach(FIL ${ARGN})
    # Make the file path absolute, so that it is also normalized (no a\..\a\b)
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    # Compute the relative path of the .proto file compared to the proto root
    FILE(RELATIVE_PATH REL_FIL ${PROTO_ROOT} ${FIL})
    # Remove the file extension
    string(REGEX REPLACE "\\.[^.]*$" "" REL_FIL_WE ${REL_FIL})

    list(APPEND SRCS "${PROTO_OUT_DIR}/${REL_FIL_WE}.cs")
 
    add_custom_command(
      OUTPUT "${PROTO_OUT_DIR}/${REL_FIL_WE}.cs"
      COMMAND protobuf::protoc
      ARGS "--proto_path=${PROTO_ROOT}" "--csharp_out=${PROTO_OUT_DIR}" ${ABS_FIL}
      DEPENDS ${ABS_FIL} protobuf::protoc
      COMMENT "Running CSharp protocol buffer compiler on ${ABS_FIL}"
      VERBATIM )

  endforeach()

  set_source_files_properties(${SRCS} PROPERTIES GENERATED TRUE)
  
  
  set(${SRCS_RET}          ${SRCS}          PARENT_SCOPE)
  set(${PROTO_OUT_DIR_RET} ${PROTO_OUT_DIR} PARENT_SCOPE)
endfunction()

# CS_FILES
# PROTO_FILES
function(PROTOBUF_ADD_CS_TO_TARGET TARGET_NAME)
set(multiValueArgs PROTO_FILES CS_FILES)

cmake_parse_arguments(INPUT "" 
  ""
  "${multiValueArgs}" ${ARGN} )

  target_sources(${TARGET_NAME} PRIVATE ${INPUT_CS_FILES})  
  
  source_group(protobuf\\cs FILES
    ${INPUT_CS_FILES} 
  )
endfunction()


function(PROTOBUF_TARGET_CSHARP TARGET_NAME PROTO_ROOT)
  set(oneValueArgs   INSTALL_FOLDER)
  cmake_parse_arguments(PROTOBUF_TARGET_CSHARP "" 
    "${oneValueArgs}"
    "" ${ARGN} )
   
  # PROTOBUF_TARGET_CSHARP_UNPARSED_ARGUMENTS: These are all .proto files
  # INPUT_INSTALL_DIR: If this option is given, we want also to install files and and add them to the interface include path.

  PROTOBUF_GENERATE_CSHARP(proto_csharp proto_out_dir ${PROTO_ROOT} ${PROTOBUF_TARGET_CSHARP_UNPARSED_ARGUMENTS})
  PROTOBUF_ADD_CS_TO_TARGET(${TARGET_NAME}
    PROTO_FILES ${PROTOBUF_TARGET_CSHARP_UNPARSED_ARGUMENTS}
    CS_FILES     ${proto_csharp}
  )
endfunction()
