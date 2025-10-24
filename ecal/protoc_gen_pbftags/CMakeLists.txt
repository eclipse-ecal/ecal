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

find_package(Protobuf REQUIRED)

add_executable(protoc-gen-pbftags)
target_sources(protoc-gen-pbftags
  PRIVATE
    main.cpp
)
target_link_libraries(protoc-gen-pbftags
  PRIVATE
    protobuf::libprotoc protobuf::libprotobuf
)

set_property(TARGET protoc-gen-pbftags PROPERTY FOLDER core)