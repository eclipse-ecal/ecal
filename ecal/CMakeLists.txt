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

cmake_minimum_required(VERSION 3.13)

set(ECAL_CORE_PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

# --------------------------------------------------------
# core
# --------------------------------------------------------
add_subdirectory(core)

if(ECAL_CORE_HAS_PROTOBUF)
  add_subdirectory(msg/protobuf)
  add_subdirectory(core_pb)
endif()

# --------------------------------------------------------
# core config
# --------------------------------------------------------
add_subdirectory(core/cfg)
if(NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(core/cfg/gen)
endif()

# --------------------------------------------------------
# services
# --------------------------------------------------------
if(ECAL_CORE_SERVICE)
  add_subdirectory(service)
endif()

# --------------------------------------------------------
# samples
# --------------------------------------------------------
if(ECAL_CORE_BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

# --------------------------------------------------------
# tests
# --------------------------------------------------------
if(ECAL_CORE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
