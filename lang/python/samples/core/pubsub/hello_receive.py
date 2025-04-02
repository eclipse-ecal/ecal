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
import ecal.core.core as ecal_core
# import the eCAL publisher AP
from ecal.core.subscriber import StringSubscriber

# Create here the eCAL receive callback. This will be called whenever a new message is received.
def callback(topic_name, msg, time):
  print("Received:  {} ms   {}".format(time, msg))


def main():
  print("-------------------------------")
  print(" Python: HELLO WORLD RECEIVER")
  print("-------------------------------")
  
  # Initialize eCAL. You always have to initialize eCAL before using it.
  # The name of our eCAL Process will be "hello_receive_python".
  # This name will be visible in the eCAL Monitor, once the process is running.
  ecal_core.initialize("hello_receive_python")
  
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
  
  # Deinitialize eCAL.
  # You should always do that before your application exits.
  ecal_core.finalize()

if __name__ == "__main__":
  main()

