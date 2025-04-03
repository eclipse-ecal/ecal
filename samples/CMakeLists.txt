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
cmake_minimum_required(VERSION 3.10)

project(eCAL_samples VERSION "${ECAL_VERSION_STRING}")

if(ECAL_USE_HDF5)
set(SAMPLE_PERSON_MEASUREMENT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/data/person)
endif(ECAL_USE_HDF5)

# --------------------------------------------------------
# cpp samples
# --------------------------------------------------------
# measurement
if(ECAL_USE_HDF5)
add_subdirectory(cpp/measurement/benchmark)
add_subdirectory(cpp/measurement/measurement_read)
add_subdirectory(cpp/measurement/measurement_write)
endif(ECAL_USE_HDF5)

# services
add_subdirectory(cpp/services/ecalplayer_client)
if(ECAL_USE_QT)
  add_subdirectory(cpp/services/ecalplayer_gui_client)
endif(ECAL_USE_QT)
add_subdirectory(cpp/services/ecalsys_client)
add_subdirectory(cpp/services/player_stepper)
add_subdirectory(cpp/services/rec_client_service_cli)
if(ECAL_USE_QT)
  add_subdirectory(cpp/services/rec_client_service_gui)
  add_subdirectory(cpp/services/rec_server_service_gui)
endif(ECAL_USE_QT)