# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2024 Continental Corporation
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

# 3.18 required for Development.Module.
# prior versions using just Development would also search for artifacts for
# embedding, which the manylinux containers don't provide
cmake_minimum_required(VERSION 3.18...3.26)

project(ecal_python)
find_package(Python REQUIRED COMPONENTS Development.Module Interpreter)

option(ECAL_PYTHON_BUILD_SAMPLES "Includes the python samples"                  OFF)
option(ECAL_PYTHON_BUILD_TESTS   "Includes the python tests"                    ON)
option(ECAL_PYTHON_USE_HDF5      "Enables eCAL application cmd line interfaces" ON)

set(ECAL_PYTHON_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# This function takes a list of python files to be copied to the package directory
# and associates the files with a given python extension target
# Using this function allows to create the whole python package editable in place
# Arguments:
#   TARGET            Python extension target with which to associate the files
#   PYTHON_CODE_ROOT  Root folder for Python files, to keep folder structure
#   PYTHON_FILES      Python files associated with target
function(copy_python_code)
  set(singleValueArgs TARGET PYTHON_CODE_ROOT)
  set(multiValueArgs  PYTHON_FILES)
  cmake_parse_arguments(ARGUMENTS
      ""
      "${singleValueArgs}"
      "${multiValueArgs}" ${ARGN} )

  cmake_path(
    ABSOLUTE_PATH   ARGUMENTS_PYTHON_CODE_ROOT 
    OUTPUT_VARIABLE absolute_path_python_code_root
  )
    
  foreach (python_file ${ARGUMENTS_PYTHON_FILES})     
    cmake_path(
      ABSOLUTE_PATH python_file 
      OUTPUT_VARIABLE absolute_path_python_file
    )
    
    cmake_path(
      RELATIVE_PATH  absolute_path_python_file 
      BASE_DIRECTORY ${absolute_path_python_code_root}
      OUTPUT_VARIABLE relative_path)
    
     
    # Now we actually copy the file to the correct directory 
    set(origin_file ${absolute_path_python_file})
    set(destination_file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/python/${relative_path})
    file(
      GENERATE 
        OUTPUT ${destination_file}
        INPUT ${origin_file}
    )
        
  endforeach()
endfunction()

# Function to set the correct output directory for the python targets, so that they can be debugged properly
function(ecal_python_set_output_directory TARGET_NAME)
  set_target_properties(${TARGET_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY $<IF:$<BOOL:${WIN32}>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/python/ecal,${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/python/ecal>
  )  
endfunction()

# Function to add a python file as a sample to a Visual Studio Solution
# It can then be started / debugged directly from within Visual Studio
# Other generators are not supported at this time.
function(ecal_python_add_sample)
  set(singleValueArgs PY_FILE TARGET_NAME)
  cmake_parse_arguments(ARGUMENTS
      ""
      "${singleValueArgs}"
      "" ${ARGN} )
           
  set(ECAL_PYPROJ_FILE              ${ARGUMENTS_PY_FILE})
  string(UUID ECAL_PYPROJ_GUID NAMESPACE 8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942 NAME ${ARGUMENTS_TARGET_NAME}
       TYPE MD5)
  get_target_property(ECAL_PYPROJ_INTERPRETER_DEBUG   Python::Interpreter LOCATION)
  get_target_property(ECAL_PYPROJ_INTERPRETER_RELEASE Python::Interpreter LOCATION)
   
  set(ECAL_PYPROJ_NAME              ${ARGUMENTS_TARGET_NAME})  
  cmake_path(GET ECAL_PYPROJ_INTERPRETER_RELEASE PARENT_PATH ECAL_PYPROJ_PYTHON_ROOT)
  set(ECAL_PYPROJ_PYTHON_VERSION    ${Python_VERSION})
  set(ECAL_PYPROJ_SEARCH_PATH_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/python)
  set(ECAL_PYPROJ_SEARCH_PATH_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/python)
  set(ECAL_PYPROJ_SEARCH_PATH_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo/python/ecal)
  
  set(generated_pyproj_path ${CMAKE_CURRENT_BINARY_DIR}/${ARGUMENTS_TARGET_NAME}.pyproj)
  
  # Generate the .pyproj file from the template
  configure_file(
    ${ECAL_PYTHON_DIRECTORY}/sample.pyproj.in
    ${generated_pyproj_path}
    @ONLY
  )

  include_external_msproject(${ARGUMENTS_TARGET_NAME} ${generated_pyproj_path})
endfunction()

# Convenience target to have all Python targets buildable via one name
add_custom_target(${PROJECT_NAME})

# We will want the shared objects to look relative to themselves for vendored
# dependencies, like eCAL core and hdf5
# NB: Even though ${ORIGIN} and $ORIGIN are both valid, auditwheel only
# understands $ORIGIN
set(CMAKE_INSTALL_RPATH "\$ORIGIN")

# Directly build with the install runtime paths as these shared objects aren't
# for build tree use.
set(CMAKE_BUILD_WITH_INSTALL_RPATH "ON")

add_subdirectory(core)
add_dependencies(${PROJECT_NAME} _ecal_core_py)

if(ECAL_USE_HDF5)
  add_subdirectory(ecalhdf5)
  add_dependencies(${PROJECT_NAME} _ecal_hdf5_py)
else()
  message(WARNING "Building Python bindings without HDF5 support")
endif()

if (ECAL_PYTHON_BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

# This is a custom target to copy the eCAL core dll to the output directory of the Python extensions.
# Without this copying step, debugging would not be possible.
if (WIN32)
    add_custom_target(copy_ecal_core_dll ALL
      COMMAND cmake -E copy_if_different "$<TARGET_FILE:eCAL::core>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/python/ecal"
      COMMENT "Copy eCAL Core DLL to python directory"
      DEPENDS eCAL::core
    )
    set_property(TARGET copy_ecal_core_dll PROPERTY FOLDER lang/python/core)
endif()