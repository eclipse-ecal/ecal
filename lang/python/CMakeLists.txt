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

add_subdirectory(nanobind_core)
add_dependencies(${PROJECT_NAME} nanobind_core)

if(HAS_HDF5)
  add_subdirectory(ecalhdf5)
  add_dependencies(${PROJECT_NAME} _ecal_hdf5_py)
else()
  message(WARNING "Building Python bindings without HDF5 support")
endif()

