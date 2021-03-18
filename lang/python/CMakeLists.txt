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

project(_ecal_py)

find_package(Python COMPONENTS Development Interpreter)
find_package(Protobuf REQUIRED)

set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
set(SETUP_PY    "${PYTHON_BINARY_DIR}/setup.py")

# Create a shortened protobuf version string
string(REPLACE "." ";" Protobuf_VERSION_components "${Protobuf_VERSION}")
list(LENGTH Protobuf_VERSION_components Protobuf_VERSION_components_length)

if ("${Protobuf_VERSION_components_length}" LESS "2")
  message(FATAL_ERROR "Unable to determine protobuf version for python binding")
endif()
list(GET Protobuf_VERSION_components 0 Protobuf_VERSION_major)
list(GET Protobuf_VERSION_components 1 Protobuf_VERSION_minor)
MATH(EXPR Protobuf_Version_minor_inc "${Protobuf_VERSION_minor}+1")

set(Protobuf_required_versions ">=${Protobuf_VERSION_major}.${Protobuf_VERSION_minor}.0,<${Protobuf_VERSION_major}.${Protobuf_Version_minor_inc}")

configure_file(${SETUP_PY_IN} ${SETUP_PY} @ONLY)

if(Python_Interpreter_FOUND)
  message(STATUS "Python interpreter found: ${Python_EXECUTABLE}")

  # on make install the python module gets installed directly by invocing python interpreter
  # TODO: this needs to be tested on windows also
  message(STATUS "Installing Python extensions")
  #install(CODE "execute_process(COMMAND ${Python_EXECUTABLE} setup.py install --user WORKING_DIRECTORY ${PYTHON_BINARY_DIR})")
  add_custom_target(create_python_wheel
      ${Python_EXECUTABLE} setup.py bdist_wheel --dist-dir=${BUILD_DEPLOY_DIRECTORY}
      DEPENDS _ecal_core_py _ecal_hdf5_py
      WORKING_DIRECTORY ${PYTHON_BINARY_DIR}
      COMMENT "Creating python wheel"
  )

  set_property(TARGET ${create_python_wheel} PROPERTY FOLDER lang/python)
else()
  message(FATAL_ERROR "Could not find python interpreter! This is needed to install the eCAL python binding!")
endif()

add_subdirectory(core)
if(HAS_HDF5)
  add_subdirectory(ecalhdf5)
endif()
