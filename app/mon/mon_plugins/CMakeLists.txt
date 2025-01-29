# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
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

add_subdirectory(plugin_template)
add_subdirectory(monitor_tree_view)
add_subdirectory(protobuf_reflection)
add_subdirectory(signals_plotting)
if (ECAL_USE_CAPNPROTO)
add_subdirectory(capnproto_reflection)
endif (ECAL_USE_CAPNPROTO)
add_subdirectory(raw_data_reflection)
add_subdirectory(string_reflection)