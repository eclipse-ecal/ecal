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
# import the eCAL subscriber API
from ecal.core.subscriber import StringSubscriber

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

