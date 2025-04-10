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

import sys
import time

import ecal.nanobind_core as ecal_core

DATA_SIZE = 1024

def my_event_callback(topic_id, callback_data):
  print("Event callback invoked")
  entity = topic_id.topic_id
  print("A subscriber with id {} from host {} with PID {} has been {}".format(entity.entity_id, entity.host_name, entity.process_id, callback_data.event_type))


def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date_string()))
  
  # initialize eCAL API
  ecal_core.initialize()

  # create publisher
  config = ecal_core.get_publisher_configuration()

  # We can assign a encoding, name or descriptor to the datatype_info.
  # However, we don't have to provide any information here
  # Specialized message publishers will set these fields
  datatype_info = ecal_core.DataTypeInformation()
  # We can additionally register an event_callback, which will be called when another entity
  # has been connected to this process
  pub = ecal_core.Publisher("blob", datatype_info, config, event_callback = my_event_callback)
  
  print(pub.get_data_type_information())
  
  # send messages
  i = 0
  while ecal_core.ok():
    filler_value = i % 256
    byte_data =  bytes([filler_value] * DATA_SIZE)
    pub.send(byte_data)
    i = i + 1
    time.sleep(0.5)
  
  # finalize eCAL API
  ecal_core.finalize()

if __name__ == "__main__":
  main()
