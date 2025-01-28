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

include(CSharpUtilities)
project(Continental.eCAL.Core.Samples LANGUAGES CSharp)

find_package(eCAL REQUIRED)

macro(ecal_add_csharp_core_sample sample_name)
  ecal_add_sample(${sample_name} ${sample_name}.cs)
  target_link_libraries(${sample_name} PRIVATE Continental.eCAL.Core)
  set_property(TARGET ${sample_name} PROPERTY FOLDER samples/csharp/core)
endmacro()

ecal_add_csharp_core_sample(pubsub_minimal_rec)
ecal_add_csharp_core_sample(pubsub_minimal_rec_cb)
ecal_add_csharp_core_sample(pubsub_minimal_snd)

ecal_add_csharp_core_sample(service_minimal_client)
ecal_add_csharp_core_sample(service_minimal_server)
