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

set(PYTHON_BINARY_DIR        ${CMAKE_BINARY_DIR}/python)
set(PYTHON_BINARY_MODULE_DIR ${CMAKE_BINARY_DIR}/python/ecal)

#! ecal_add_python_module : this function adds a python module
#
# This function
#
# \arg:TARGET_NAME the first argument
# \param:SOURCES SOURCES specify the fooness of the function
# \param:PYTHON_CODE PYTHON_CODE should always be 42
# \group:GROUP1 GROUP1 is a list of project to foo
#
function(ecal_add_python_module TARGET_NAME)

  set(multiValueArgs SOURCES)
  set(singleValueArgs PYTHON_CODE)
  cmake_parse_arguments(ARGUMENTS
      ""
      "${singleValueArgs}"
      "${multiValueArgs}" ${ARGN} )

  if(NOT ARGUMENTS_SOURCES AND NOT ARGUMENTS_PYTHON_CODE)
    message(ERROR "Error in ecal_add_python_module: Please specify SOURCES and / or PYTHON_CODE arguments")
  endif()


  # if Sources are specified, a library is created
  if(ARGUMENTS_SOURCES)
    Python_add_library(${TARGET_NAME} MODULE ${ARGUMENTS_SOURCES})
    set_target_properties(${TARGET_NAME}
        PROPERTIES
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PYTHON_BINARY_MODULE_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PYTHON_BINARY_MODULE_DIR}"        
        LIBRARY_OUTPUT_DIRECTORY "${PYTHON_BINARY_MODULE_DIR}"
        DEBUG_POSTFIX "_d"
        )
  # if no sources are specified, then a custom target needs to be created
  else()
    add_custom_target(${TARGET_NAME} ALL
        COMMENT "Custom python target: ${TARGET_NAME}")
  endif()

  if(ARGUMENTS_PYTHON_CODE)
    # Copy all files from the source folder to the python binary directory.
    get_filename_component(absolute_folder_python_files ${ARGUMENTS_PYTHON_CODE} ABSOLUTE)
    file(GLOB_RECURSE relative_python_files
     RELATIVE ${absolute_folder_python_files}
     LIST_DIRECTORIES false
     CONFIGURE_DEPENDS
     ${absolute_folder_python_files}/*.py)
     
    foreach (f ${relative_python_files})  
      set(origin_file ${absolute_folder_python_files}/${f})
      set(destination_file ${PYTHON_BINARY_MODULE_DIR}/${f})
      configure_file(${origin_file} ${destination_file} COPYONLY) 
    endforeach()
  endif()
endfunction()

function(ecal_add_pybind11_module TARGET_NAME)
  set(multiValueArgs SOURCES)
  set(singleValueArgs PYTHON_CODE)
  cmake_parse_arguments(ARGUMENTS
      ""
      "${singleValueArgs}"
      "${multiValueArgs}" ${ARGN} )

  if(NOT ARGUMENTS_SOURCES AND NOT ARGUMENTS_PYTHON_CODE)
    message(ERROR "Error in ecal_add_python_module: Please specify SOURCES and / or PYTHON_CODE arguments")
  endif()
  
  
  pybind11_add_module(${TARGET_NAME} ${ARGUMENTS_SOURCES})
  set_target_properties(${TARGET_NAME}
    PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PYTHON_BINARY_MODULE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PYTHON_BINARY_MODULE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PYTHON_BINARY_MODULE_DIR}"
    DEBUG_POSTFIX "_d"
  )
  
  if(ARGUMENTS_PYTHON_CODE)
    add_custom_command(
        TARGET ${TARGET_NAME}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ARGUMENTS_PYTHON_CODE} ${PYTHON_BINARY_MODULE_DIR}
    )
  endif()
endfunction ()

function(ecal_install_python_module)
endfunction()