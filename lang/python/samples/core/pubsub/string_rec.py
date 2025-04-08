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

import sys

import ecal.core.core as ecal_core
from ecal.core.subscriber import StringSubscriber

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # initialize eCAL API
  ecal_core.initialize("py_minimal_rec")
  
  # set process state
  ecal_core.set_process_state(1, 1, "I feel good")

  # create subscriber
  sub = StringSubscriber("hello")
  
  # receive messages
  while ecal_core.ok():
    ret, msg, time = sub.receive(500)
    if ret > 0:
      print("Received:  {} ms   {}".format(time, msg))
    else:
      print("Subscriber timeout ..")
  
  # finalize eCAL API
  ecal_core.finalize()

if __name__ == "__main__":
  main()

