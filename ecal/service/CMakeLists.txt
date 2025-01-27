# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2023 Continental Corporation
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

cmake_minimum_required(VERSION 3.16)
project(ecal_service)

# Root CMakeLists sets CMAKE_MODULE_PATH to include the correct path
# If the module cannot be found warnings basic warnings will be applied instead
include(helper_functions/ecal_compiler_warnings OPTIONAL RESULT_VARIABLE _ecal_warnings_found)
if("${_ecal_warnings_found}" STREQUAL "NOTFOUND")
  message(AUTHOR_WARNING
    "  helper_functions/ecal_compiler_warnings.cmake not found.\n"
    "  Add the path to CMAKE_MODULE_PATH to use warnings specified by the main eCAL repo."
  )
  add_library(_ecal_service_warnings INTERFACE)
  set(cxx_compiler_flags "")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
      set(cxx_compiler_flags "/MP" "/W4")
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
      set(cxx_compiler_flags "-Wall" "-Wextra")
  else()
      message(WARNING "Unknown compiler, will not set warning flags")
  endif()
  target_compile_options(_ecal_service_warnings INTERFACE "${cxx_compiler_flags}")
else()
  add_library(_ecal_service_warnings ALIAS _ecal_warnings)
endif()

# Main library
add_subdirectory(ecal_service)

# Samples
if(ECAL_CORE_BUILD_SAMPLES)
  add_subdirectory(samples/sample_client)
  add_subdirectory(samples/sample_server)
  add_subdirectory(samples/sample_standalone)
endif()

# Tests
if(ECAL_CORE_BUILD_TESTS)
  add_subdirectory(test)
endif()
