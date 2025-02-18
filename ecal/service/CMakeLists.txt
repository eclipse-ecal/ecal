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

if(TARGET _ecal_warnings)
  # _ecal_warnings is the internal eCAL target containing the warning flags
  # and will be absent if this is being built standalone
  add_library(_ecal_service_warnings ALIAS _ecal_warnings)
else()
  # Otherwise, use a fallback implementation
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(cxx_compiler_flags "/MP" "/W4")
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
    set(cxx_compiler_flags "-Wall" "-Wextra")
  else()
    message(WARNING "Unknown compiler, will not set warning flags")
  endif()

  set(ECAL_SERVICE_COMPILER_WARNINGS "${cxx_compiler_flags}" CACHE STRING "Warning flags used for eCAL service code")
  add_library(_ecal_service_warnings INTERFACE)
  target_compile_options(_ecal_service_warnings INTERFACE "${ECAL_SERVICE_COMPILER_WARNINGS}")
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
