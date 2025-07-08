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
import random
import time

# import the eCAL core API
import ecal.nanobind_core as ecal_core

DATA_SIZE = 16

def random_printable_bytes(length: int) -> bytes:
    """
    Generate a bytes object of the given length, where each byte is
    a randomly chosen printable ASCII character (code points 32–126).

    :param length: Number of bytes in the resulting bytes object.
    :return: bytes of size `length`, with each element in [32,126].
    """
    return bytes(random.randint(32, 126) for _ in range(length))

def main():
  print("----------------------")
  print(" Python: BLOB SENDER  ")
  print("----------------------")
  
  # Initialize eCAL. You always have to initialize eCAL before using it.
  # The name of our eCAL Process will be "hello send python".
  # This name will be visible in the eCAL Monitor, once the process is running.
  ecal_core.initialize("blob send python")

  # Print used eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date_string()))
 
  # Set the state for the program.
  # You can vary between different states like healthy, warning, critical ...
  # This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  ecal_core.process.set_state(ecal_core.process.Severity.HEALTHY, ecal_core.process.SeverityLevel.LEVEL1, "I feel good!")

  # Now we create a new publisher that will publish the topic "blob".
  pub = ecal_core.Publisher("blob")
  
  # Creating an infinite publish-loop.
  # eCAL Supports a stop signal; when an eCAL Process is stopped, ecal_core.ok() will return false.
  while ecal_core.ok():
    # Generate a random message of DATA_SIZE bytes
    # Each byte is a randomly chosen printable ASCII character (code points 32–126).    
    byte_data = random_printable_bytes(DATA_SIZE)

    # Send the message. The message is sent to all subscribers that are currently connected to the topic "blob".  
    pub.send(byte_data)

    # Sleep for 500 ms so we send with a frequency of 2 Hz.
    time.sleep(0.5)
  
  # Finalize eCAL.
  # You should always do that before your application exits.
  ecal_core.finalize()

if __name__ == "__main__":
  main()
