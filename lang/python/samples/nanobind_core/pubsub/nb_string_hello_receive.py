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
# import the eCAL core API
import ecal.nanobind_core as ecal_core
# import the eCAL subscriber API
from ecal.msg.string.core import Subscriber as StringSubscriber

# Create here the eCAL receive callback. This will be called whenever a new message is received.
def callback(topic_name, msg, time):
  print("Received:  {} ms   {}".format(time, msg))


def main():
  print("-------------------------------")
  print(" Python: HELLO WORLD RECEIVER")
  print("-------------------------------")
  
  # Initialize eCAL. You always have to initialize eCAL before using it.
  # The name of our eCAL Process will be "hello receive python".
  # This name will be visible in the eCAL Monitor, once the process is running.
  ecal_core.initialize("hello receive python")
  
  # Print used eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # Set the state for the program.
  # You can vary between different states like healthy, warning, critical ...
  # This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  ecal_core.set_process_state(1, 1, "I feel good!")
  
  # Creating the eCAL Subscriber. An eCAL Process can create multiple subscribers (and publishers).
  # The topic we are going to receive is called "hello".
  sub = StringSubscriber("hello")
  
  # Register the callback with the subscriber so it can be called.
  sub.set_callback(callback)
  
  # Creating an infinite loop.
  # eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
  while ecal_core.ok():
    # Sleep for 500ms to avoid busy waiting.
    # You can use eCAL::Process::SleepMS() to sleep in milliseconds.
    time.sleep(0.5)
  
  # Finalize eCAL.
  # You should always do that before your application exits.
  ecal_core.finalize()

if __name__ == "__main__":
  main()


import sys
import time



# eCAL receive callback
def data_callback(publisher_id : ecal_core.TopicId, datatype_info : ecal_core.DataTypeInformation, data : ecal_core.ReceiveCallbackData):
  output = f"""
  ----------------------------------------------
   Received binary buffer in Python
  ----------------------------------------------
  Size         : {len(data.buffer)}
  Time         : {data.send_timestamp}
  Clock        : {data.send_clock}
  Content      : {data.buffer}
  """
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

