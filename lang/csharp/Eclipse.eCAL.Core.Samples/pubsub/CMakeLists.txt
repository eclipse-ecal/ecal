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

include(CSharpUtilities)
project(Eclipse.eCAL.Core.Samples LANGUAGES CSharp)

find_package(eCAL REQUIRED)

macro(ecal_add_csharp_core_sample sample_name)
  ecal_add_sample(${sample_name} ${sample_name}.cs)
  target_link_libraries(${sample_name} PRIVATE Eclipse.eCAL.Core)
  set_property(TARGET ${sample_name} PROPERTY FOLDER samples/csharp/binary/pubsub)
endmacro()

ecal_add_csharp_core_sample(minimal_events_rec_csharp)
ecal_add_csharp_core_sample(minimal_events_snd_csharp)
ecal_add_csharp_core_sample(blob_send_csharp)
ecal_add_csharp_core_sample(blob_receive_csharp)
