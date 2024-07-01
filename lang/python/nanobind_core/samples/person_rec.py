# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2024 Continental Corporation
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

import nanobind_core as ecal_core
from nb_subscriber import ProtoSubscriber

sys.path.insert(1, os.path.join(sys.path[0], '../_protobuf'))
import person_pb2

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date()))
  
  # initialize eCAL API
  ecal_core.initialize()
  
  # set process state
  #ecal_core.set_process_state(1, 1, "I feel good")

  # create publisher
  sub = ProtoSubscriber("person", person_pb2.Person)
  
  # receive messages
  while ecal_core.ok():
    ret, person, time = sub.receive(500)
  
    # deserialize person from message buffer
    if ret > 0:
      # print person content
      print("")
      print("Received person ..")
      print("person id    : {}".format(person.id))
      print("person name  : {}".format(person.name))
      print("person stype : {}".format(person.stype))
      print("person email : {}".format(person.email))
      print("dog.name     : {}".format(person.dog.name))
      print("house.rooms  : {}".format(person.house.rooms))
  
  print("No reception")
  # finalize eCAL API
  ecal_core.finalize()
  
if __name__ == "__main__":
  main()  
