# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2025 Continental Corporation
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

# C build, can be executed standalone against an eCAL installation
project(ecal_c)

option(ECAL_C_BUILD_SAMPLES "BUILD eCAL C samples" ON)
option(ECAL_C_BUILD_TESTS   "BUILD eCAL C unittests" ON)

add_subdirectory(core)

if (ECAL_C_BUILD_SAMPLES)
add_subdirectory(samples)
endif()

if (ECAL_C_BUILD_TESTS)
enable_testing()
add_subdirectory(tests)
endif ()