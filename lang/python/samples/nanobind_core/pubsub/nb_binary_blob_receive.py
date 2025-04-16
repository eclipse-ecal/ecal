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

# eCAL receive callback
def data_callback(publisher_id : ecal_core.TopicId, datatype_info : ecal_core.DataTypeInformation, data : ecal_core.ReceiveCallbackData):
  output = """
  ----------------------------------------------
   Received binary buffer
  ----------------------------------------------
  Size         : {}
  Time         : {}
  Clock        : {}
  """.format(len(data.buffer), data.send_timestamp, data.send_clock)
  print(output)

def publisher_event_callback(publisher_id : ecal_core.TopicId, callback_data : ecal_core.SubEventCallbackData):
  print("Event callback invoked")
  entity = publisher_id.topic_id
  print("A publisher with id {} from host {} with PID {} has been {}".format(entity.entity_id, entity.host_name, entity.process_id, callback_data.event_type))
  
def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date_string()))
  
  # initialize eCAL API
  ecal_core.initialize("blob receive python")

  # create subscriber
  config = ecal_core.get_subscriber_configuration()

  # We can assign a encoding, name or descriptor to the datatype_info.
  # However, we don't have to provide any information here
  # Specialized message publishers will set these fields
  datatype_info = ecal_core.DataTypeInformation()
  # create subscriber and connect callback
  sub = ecal_core.Subscriber("blob", datatype_info, config, event_callback = publisher_event_callback)
  sub.set_receive_callback(data_callback)
  
  print(sub.get_data_type_information())
  
  # idle main thread
  while ecal_core.ok():
    time.sleep(0.1)
  
  # finalize eCAL API
  ecal_core.finalize()
  
if __name__ == "__main__":
  main()

