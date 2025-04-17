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
# import the eCAL publisher API supporting protobuf
from ecal.core.publisher import ProtoPublisher

sys.path.insert(1, os.path.join(sys.path[0], '../_protobuf'))
# import the compiled protobuf message classes
import person_pb2

def main():
  print("-----------------------")
  print(" Python: PERSON SENDER")
  print("-----------------------")
  
  # Initialize eCAL. You always have to initialize eCAL before using it.
  # The name of our eCAL Process will be "person send python".
  # This name will be visible in the eCAL Monitor, once the process is running.
  ecal_core.initialize("person send python")
  
  # Print used eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))

  # Set the state for the program.
  # You can vary between different states like healthy, warning, critical ...
  # This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  ecal_core.set_process_state(1, 1, "I feel good!")

  # Creating the eCAL Publisher. An eCAL Process can create multiple publishers (and subscribers).
  # The topic we are going to publish is called "person".
  # Furthermore, we want to tell the publisher that we want to use the protobuf message class "Person" from the file "person.proto".
  pub = ProtoPublisher("person", person_pb2.Person)
  
  # Create a new message object of type "Person" and fill it with some data.
  person = person_pb2.Person()
  person.name        = "Max"
  person.stype       = person_pb2.Person.MALE
  person.email       = "max@mail.net"
  person.dog.name    = "Brandy"
  person.house.rooms = 4
  
  # Creating an inifite publish-loop.
  # eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
  loop_count = 0
  while ecal_core.ok():
    
    # Change something in the message so we can see that the message is changing, e.g. in eCAL Monitor.
    loop_count = loop_count + 1
    person.id = loop_count
  
    # Send the content to other eCAL Processes that have subscribed to the topic "person".
    pub.send(person)
    print("Sending person {}".format(person.id))
  
    # Sleep for 500 ms so we send with a frequency of 2 Hz.
    time.sleep(0.5)
  
  # Finalize eCAL.
  # You should always do that before your application exits.
  ecal_core.finalize()
  
if __name__ == "__main__":
  main()  
