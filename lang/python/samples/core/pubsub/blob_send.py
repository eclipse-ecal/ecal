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

def main():
  print("---------------------")
  print(" Python: BLOB SENDER")
  print("---------------------")
  
  # Initialize eCAL. You always have to initialize eCAL before using it.
  # The name of our eCAL Process will be "blob send python".
  # This name will be visible in the eCAL Monitor, once the process is running.
  ecal_core.initialize("blob send python")
  
  # Print used eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # Set the state for the program.
  # You can vary between different states like healthy, warning, critical ...
  # This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  ecal_core.set_process_state(1, 1, "I feel good")

  # Creating the eCAL Publisher. An eCAL Process can create multiple publishers (and subscribers).
  # The topic we are going to publish is called "blob".
  pub = BinaryPublisher("blob")
  
  # We create a message to send that will be altered later to notice different messages.
  msg_fox = b"4120717569636b2062726f776e20666f7820"
  
  loop_count = 0
  # Creating an inifite publish-loop.
  # eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL::Ok() will return false.
  while ecal_core.ok():
    # Prepare the blob to send
    loop_count += 1
    hex_ascii = bytes(''.join([hex(ord(digit))[2:] for digit in str(loop_count)]), "utf-8")
    msg = msg_fox + hex_ascii
    
    # Send the content to other eCAL Processes that have subscribed to the topic "blob".
    pub.send(msg)
    print("Sent: ", msg)
    
    # Sleep for 500 ms so we send with a frequency of 2 Hz.
    time.sleep(0.5)
  
  # Finalize eCAL.
  # You should always do that before your application exits.
  ecal_core.finalize()

if __name__ == "__main__":
  main()