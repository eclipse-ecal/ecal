# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2021 Continental Corporation
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

# This file provides functions to generate files through protoc.exe
# Unlike PROTOBUF_GENERATE_CPP the function can handle files provided in a directory structure, however
# you will have to provide the source folder to this structure.

function(PROTOBUF_GENERATE_PYTHON_EXT SRCS_RET PROTO_OUT_DIR PROTO_ROOT)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_PYTHON_EXT() called without any proto files")
    return()
  endif()
  
  if (POLICY CMP0057)
    cmake_policy(SET CMP0057 NEW) # set policy to enable IN_LIST operator
  endif(POLICY CMP0057)
  
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


  file(MAKE_DIRECTORY ${PROTO_OUT_DIR})  
  get_filename_component(PROTO_ROOT ${PROTO_ROOT} ABSOLUTE)
    
  foreach(FIL ${ARGN})
    # Make the file path absolute, so that it is also normalized (no a\..\a\b)
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    # Compute the relative path of the .proto file compared to the proto root
    FILE(RELATIVE_PATH REL_FIL ${PROTO_ROOT} ${FIL})
    # Remove the file extension
    string(REGEX REPLACE "\\.[^.]*$" "" REL_FIL_WE ${REL_FIL})
    list(APPEND SRCS "${PROTO_OUT_DIR}/${REL_FIL_WE}_pb2.py")
    
    #put relative folders into directory list if not in there yet
    get_filename_component(REL_FIL_DIRECTORY ${REL_FIL} DIRECTORY)
    if (NOT "${REL_FIL_DIRECTORY}" IN_LIST REL_FOLDERS)
      list(APPEND REL_FOLDERS ${REL_FIL_DIRECTORY})
    endif()
    
    add_custom_command(
      OUTPUT "${PROTO_OUT_DIR}/${REL_FIL_WE}_pb2.py"
      COMMAND protobuf::protoc
      ARGS "--proto_path=${PROTO_ROOT}" "--python_out=${PROTO_OUT_DIR}" ${ABS_FIL}
      DEPENDS ${ABS_FIL} protobuf::protoc
      COMMENT "Running python protocol buffer compiler on ${ABS_FIL}: $<TARGET_FILE:protobuf::protoc>"
      VERBATIM )

  endforeach()

  set_source_files_properties(${SRCS} PROPERTIES GENERATED TRUE)
  _generate_init_py(${PROTO_OUT_DIR} "${REL_FOLDERS}")
  set(${SRCS_RET} ${SRCS} PARENT_SCOPE)
endfunction()

# This function generates __init__.py files in all directories from proto root
# which will be created
# e.g. 
# pb
#  a.proto
#  sub
#    sub.proto
#
# Generated file structure in PROTO_OUT_DIR
# pb
#   __init__.py
#   a_pb2.py
#   sub
#     __init__.py
#     sub_pb2.py
#
function(_generate_init_py PROTO_OUT_DIR RELATIVE_DIRECTORY_LIST)

foreach(directory ${RELATIVE_DIRECTORY_LIST})
file(GENERATE OUTPUT ${PROTO_OUT_DIR}/${directory}/__init__.py
  CONTENT ""
)
endforeach()

endfunction()
