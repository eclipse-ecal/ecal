# ========================= eCAL LICENSE =================================
#
# Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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


#Unfortunately, protozero currently does not support targets, so we will define them

include_guard(GLOBAL)

include(GNUInstallDirs)
add_library(protozero INTERFACE EXCLUDE_FROM_ALL)
target_include_directories(protozero INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/protozero/include>
)
add_library(protozero::protozero ALIAS protozero)

# We don't want to install protozero. However we need to have the commands
# Otherwise CMake will not be happy
install(
  TARGETS protozero
  EXPORT protozeroTargets
)

install(
  EXPORT protozeroTargets 
  FILE protozeroTargets.cmake 
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake"
  NAMESPACE protozero::
  COMPONENT protozero_dev
)
