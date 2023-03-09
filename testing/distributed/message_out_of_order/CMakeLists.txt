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

project(message_out_of_order)

find_package(Threads REQUIRED)

add_executable(message_out_of_order_publisher publisher.cpp)
add_executable(message_out_of_order_lazy_subscriber lazy_subscriber.cpp)
add_executable(message_out_of_order_crazy_subscriber crazy_subscriber.cpp)

foreach(executable message_out_of_order_publisher message_out_of_order_lazy_subscriber message_out_of_order_crazy_subscriber)
  target_link_libraries(${executable}
    PRIVATE
      eCAL::core
      Threads::Threads)
  target_compile_features(${executable} PRIVATE cxx_std_14)
  set_property(TARGET ${executable} PROPERTY FOLDER testing/distributed)
endforeach()

