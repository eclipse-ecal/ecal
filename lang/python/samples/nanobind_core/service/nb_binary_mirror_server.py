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

import time
import textwrap
from typing import Tuple

import ecal.nanobind_core as ecal_core

def print_data(method_name : str, request : bytes, response : bytes):
  print(f"Method   : '{method_name}' called in Python")
  print(f"Requests : {request}")
  print(f"Response : {response}")
  print()

# Define the callback function that will be called when a client calls the service method "echo".
def echo_req_callback(
    method_information : ecal_core.ServiceMethodInformation,
    request : bytes) -> Tuple[int, bytes]:
  response = request  
  print_data(method_information.method_name, request, response)
  return 0, response

# Define the callback function that will be called when a client calls the service method "reverse".
def reverse_req_callback(
    method_information : ecal_core.ServiceMethodInformation,
    request : bytes) -> Tuple[int, bytes]:
  response = request[::-1] #reverse the request
  print_data(method_information.method_name, request, response)
  return 0, response

def main():
  print(textwrap.dedent("""
    -----------------------
     Python: MIRROR SERVER
    -----------------------
  """).strip())
          
  # First, initialize the eCAL API.
  ecal_core.initialize("mirror server python")

  # Print eCAL version and date.
  print(f"eCAL {ecal_core.get_version_string()} ({ecal_core.get_version_date_string()})")
  ecal_core.process.set_state(ecal_core.process.Severity.HEALTHY, ecal_core.process.SeverityLevel.LEVEL1, "I feel good");

  # Create the mirror server and give it the name "mirror".
  server = ecal_core.ServiceServer("mirror")

  # The server will have two methods: "echo" and "reverse".
  # To set a callback, we need to set a ServiceMethodInformation struct as well as the callback function.
  echo_method_info = ecal_core.ServiceMethodInformation(method_name="echo")
  server.set_method_callback(echo_method_info, echo_req_callback)
  reverse_method_info = ecal_core.ServiceMethodInformation(method_name="reverse")
  server.set_method_callback(reverse_method_info, reverse_req_callback)
  
  # We can retrieve the services name / id.
  print(server.get_service_name())
  print(server.get_service_id())

  # We can check if clients have connected to this service.
  print(server.is_connected())

  # Now we will go in an infinite loop, to wait for incoming service calls that will be handled with the callbacks.
  while(ecal_core.ok()):
    time.sleep(0.1)

  # destroy server
  server.destroy()
  
  # finalize eCAL API
  server.finalize()

if __name__ == "__main__":
  main()
