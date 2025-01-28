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

cmake_minimum_required(VERSION 3.13)

project(eCAL_String)

option(ECAL_STRING_BUILD_SAMPLES "BUILD eCAL String samples"   ON)
option(ECAL_STRING_BUILD_TESTS   "BUILD eCAL String unittests" ON)

# --------------------------------------------------------
# main extension functionality
# --------------------------------------------------------
add_subdirectory(string)

# --------------------------------------------------------
# samples
# --------------------------------------------------------
if(ECAL_STRING_BUILD_SAMPLES)
  add_subdirectory(samples/pubsub/minimal_rec)
  add_subdirectory(samples/pubsub/minimal_snd)
endif()

# --------------------------------------------------------
# tests
# --------------------------------------------------------
if(ECAL_STRING_BUILD_TESTS)
  enable_testing()
endif()