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

import os
import sys
import time
import typing
# import the eCAL core API
import ecal.nanobind_core as ecal_core
# import the eCAL subscriber API
from ecal.msg.proto.core import Subscriber as ProtobufSubscriber
from ecal.msg.common.core import ReceiveCallbackData

import google.protobuf
from packaging import version
proto_version = version.parse(google.protobuf.__version__)
sys.path.insert(1, os.path.join(sys.path[0], f"./_protobuf/v{proto_version.major}"))
# import the compiled protobuf message classes
import person_pb2

# eCAL receive callback
def data_callback(publisher_id : ecal_core.TopicId, data : ReceiveCallbackData[person_pb2.Person]) -> None:
  output = f"""
  ----------------------------------------------
   Received person message from topic {publisher_id.topic_name} in Python
  ----------------------------------------------
  Time         : {data.send_timestamp}
  Clock        : {data.send_clock}
  Message      : 
    {data.message}
  """
  print(output)

def main():
  print("--------------------------------")
  print(" Python:  PERSON RECEIVE        ")
  print("--------------------------------")

  # Initialize eCAL. You always have to initialize eCAL before using its API.
  # The name of our eCAL Process will be "hello receive python". 
  # This name will be visible in the eCAL Monitor, once the process is running.
  ecal_core.initialize("person receive python")

  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date_string()))

  # Set the state for the program.
  # You can vary between different states like healthy, warning, critical ...
  # This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  ecal_core.process.set_state(ecal_core.process.Severity.HEALTHY, ecal_core.process.SeverityLevel.LEVEL1, "I feel good!")

  # Creating the eCAL Subscriber. An eCAL Process can create multiple subscribers (and publishers).
  # The topic we are going to receive is called "person".
  sub = ProtobufSubscriber[person_pb2.Person](person_pb2.Person, "person")
  sub.set_receive_callback(data_callback)
  
  # Creating an infinite loop.
  # eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
  while ecal_core.ok():
    time.sleep(0.5) # Sleep for 500ms to reduce CPU usage.
  
  # Finalize eCAL.
  # You should always do that before your application exits.
  ecal_core.finalize()
  
if __name__ == "__main__":
  main()

