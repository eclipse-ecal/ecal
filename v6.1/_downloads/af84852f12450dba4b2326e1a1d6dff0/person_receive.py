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
# import the eCAL core API
import ecal.core.core as ecal_core
# import the eCAL subscriber API
from ecal.core.subscriber import ProtoSubscriber

sys.path.insert(1, os.path.join(sys.path[0], '../_protobuf'))
# import the protobuf generated classes
import person_pb2

# Create here the eCAL receive callback. This will be called whenever a new message is received.
def callback(topic_name, person, time):
  print("")
  print("Received person ..")
  print("person id    : {}".format(person.id))
  print("person name  : {}".format(person.name))
  print("person stype : {}".format(person.stype))
  print("person email : {}".format(person.email))
  print("dog.name     : {}".format(person.dog.name))
  print("house.rooms  : {}".format(person.house.rooms))

def main():
  print("----------------------------------")
  print(" Python (legacy): PERSON RECEIVER ")
  print("----------------------------------")

  # Initialize eCAL. You always have to initialize eCAL before using it.
  # The name of our eCAL Process will be "person receive python (legacy)".
  # This name will be visible in the eCAL Monitor, once the process is running.
  ecal_core.initialize("person receive python (legacy)")

  # Print used eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # Set the state for the program.
  # You can vary between different states like healthy, warning, critical ...
  # This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  ecal_core.set_process_state(1, 1, "I feel good!")
  
  # Creating the eCAL Subscriber. An eCAL Process can create multiple subscribers (and publishers).
  # The topic we are going to receive is called "person".
  # The topic type is specified by the protobuf message class.
  sub = ProtoSubscriber("person", person_pb2.Person)
  
  # Register the callback with the subscriber so it can be called.
  # The callback will be called whenever a new message is received.
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
