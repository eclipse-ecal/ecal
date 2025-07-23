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
import random

# Import the eCAL core API
import ecal.core.core as ecal_core
# Import the eCAL binary publisher API
from ecal.core.publisher import BinaryPublisher


def generate_binary_message(buffer_size=16):
  random.seed(time.time())
  return ''.join(chr(random.randint(32, 126)) for _ in range(buffer_size))


def main():
  print("-------------------------------")
  print(" Python (legacy): BLOB SENDER")
  print("-------------------------------")
  
  # Initialize eCAL. You always have to initialize eCAL before using it.
  # The name of our eCAL Process will be "blob send python (legacy)".
  # This name will be visible in the eCAL Monitor, once the process is running.
  ecal_core.initialize("blob send python (legacy)")
  
  # Print used eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # Set the state for the program.
  # You can vary between different states like healthy, warning, critical ...
  # This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  ecal_core.set_process_state(1, 1, "I feel good!")

  # Creating the eCAL Publisher. An eCAL Process can create multiple publishers (and subscribers).
  # The topic we are going to publish is called "blob".
  pub = BinaryPublisher("blob")
   
  loop_count = 0
  # Creating an inifite publish-loop.
  # eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
  while ecal_core.ok():
    # Generate a random message of 16 bytes
    msg = generate_binary_message(16)
    
    # Send the content to other eCAL Processes that have subscribed to the topic "blob".
    pub.send(msg)
    print("Sent: ", msg)
    
    loop_count += 1
    
    # Sleep for 500 ms so we send with a frequency of 2 Hz.
    time.sleep(0.5)
  
  # Finalize eCAL.
  # You should always do that before your application exits.
  ecal_core.finalize()

if __name__ == "__main__":
  main()