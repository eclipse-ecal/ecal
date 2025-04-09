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
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # Initialize eCAL. You always have to initialize eCAL before using it.
  # The name of our eCAL Process will be "python_person_send".
  # This name will be visible in the eCAL Monitor, once the process is running.
  ecal_core.initialize("person_send_python")
  
  # set process state
  ecal_core.set_process_state(1, 1, "I feel good")

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
  while ecal_core.ok():
  
    # change person id
    person.id = person.id + 1
  
    # send person
    print("Sending person {}".format(person.id))
    pub.send(person)
  
    # sleep 100 ms
    time.sleep(0.1)
  
  # finalize eCAL API
  ecal_core.finalize()
  
if __name__ == "__main__":
  main()  
