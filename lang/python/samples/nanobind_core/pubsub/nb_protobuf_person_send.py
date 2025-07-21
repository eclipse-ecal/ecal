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

import random
import time

import ecal.nanobind_core as ecal_core

DATA_SIZE = 16

# This function will be called when a subscriber is connected / disconnected to the publisher
def subscriber_event_callback(subscriber_id : ecal_core.TopicId, callback_data : ecal_core.PubEventCallbackData):
  print("Event callback invoked")
  entity = subscriber_id.topic_id
  print("A subscriber with id {} from host {} with PID {} has been {}".format(entity.entity_id, entity.host_name, entity.process_id, callback_data.event_type))

def random_printable_bytes(length: int) -> bytes:
    """
    Generate a bytes object of the given length, where each byte is
    a randomly chosen printable ASCII character (code points 32–126).

    :param length: Number of bytes in the resulting bytes object.
    :return: bytes of size `length`, with each element in [32,126].
    """
    return bytes(random.randint(32, 126) for _ in range(length))

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date_string()))
  
  # initialize eCAL API
  ecal_core.initialize("blob send python")

  # create publisher
  config = ecal_core.get_publisher_configuration()

  # We can assign a encoding, name or descriptor to the datatype_info.
  # However, we don't have to provide any information here
  # Specialized message publishers will set these fields
  datatype_info = ecal_core.DataTypeInformation()
  # We can additionally register an event_callback, which will be called when another entity
  # has been connected to this process
  pub = ecal_core.Publisher("blob", datatype_info, config, event_callback = subscriber_event_callback)
  
  # Let's sleep before we start sending data, so other processes can connect
  time.sleep(2)

  print(pub.get_data_type_information())
  
  # send messages
  while ecal_core.ok():
    byte_data = random_printable_bytes(DATA_SIZE)
    pub.send(byte_data)
    time.sleep(0.5)
  
  # finalize eCAL API
  ecal_core.finalize()

if __name__ == "__main__":
  main()
