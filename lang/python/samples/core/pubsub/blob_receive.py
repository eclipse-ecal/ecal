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
import time

import ecal.core.core as ecal_core
from ecal.core.subscriber import BinarySubscriber

# eCAL receive callback
def callback(topic_name, msg, time):
  print("Received:  {} ms   {}".format(time, bytes.fromhex(msg.decode("utf-8"))))

def main():  
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # initialize eCAL API
  ecal_core.initialize("py_binary_rec_cb")
  
  # set process state
  ecal_core.set_process_state(1, 1, "I feel good")

  # create subscriber and connect callback
  sub = BinarySubscriber("Hello")
  sub.set_callback(callback)
  
  # idle main thread
  while ecal_core.ok():
    time.sleep(0.1)
  
  # finalize eCAL API
  ecal_core.finalize()
  
if __name__ == "__main__":
  main()

