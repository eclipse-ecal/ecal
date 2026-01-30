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

from pickle import TRUE
import sys
import textwrap
import time

import ecal.nanobind_core as ecal_core

def print_service_response(service_response : ecal_core.ServiceResponse):
  print(f"Received service response in Python: {service_response.call_state}")
  print(f"Method    : {service_response.service_method_information.method_name}")
  print(f"Response  : {service_response.response}")
  print(f"Server ID : {service_response.server_id.service_id.entity_id}")
  print(f"Host      : {service_response.server_id.service_id.host_name}")
  print()

# generator, alternating returns "echo", "reverse"
def generate_method_name():
  methods = ["echo", "reverse"]
  while True:
    for method in methods:
      yield method 

def main():
  print(textwrap.dedent("""
    -----------------------
     Python: MIRROR CLIENT
    -----------------------
  """).strip())
          
  # First, initialize the eCAL API.
  ecal_core.initialize("mirror client python")

  # Print eCAL version and date.
  print(f"eCAL {ecal_core.get_version_string()} ({ecal_core.get_version_date_string()})")
  ecal_core.process.set_state(ecal_core.process.Severity.WARNING, ecal_core.process.SeverityLevel.LEVEL1, "Waiting for a server");
  
  # Create a client for the "mirror" service
  mirror_client = ecal_core.ServiceClient("mirror")
  
  # Wait until the client is connected to a server,
  # so we don't call methods that are not available.
  while (not mirror_client.is_connected()):
    print("Waiting for a service ...")
    time.sleep(1.0)

  # Now that we are connected, we can set the process state to "healthy" and communicate the connection.
  ecal_core.process.set_state(ecal_core.process.Severity.HEALTHY, ecal_core.process.SeverityLevel.LEVEL1, "Connected!");
  
  # instantiate the generator to switch between calls to "echo" and "reverse".
  method_generator = generate_method_name()

  # idle and call service methods
  while(ecal_core.ok()):

    # We iterate now over all client instances and call the methods by name.
    # With this approach we have the option to filter out client instances that we don't want to call.

    service_request = bytes("stressed", "ascii")
    service_method_name = next(method_generator)

    client_instances = mirror_client.get_client_instances()
    for client_instance in client_instances:
        
        # There are three ways that a client can call the server
        # 1. Blocking, with a response 
        (ok, response) = client_instance.call_with_response(service_method_name, service_request, timeout_ms=1000)
        if (response):
          print_service_response(response)
        time.sleep(0.5)

        # 2. Blocking with callback
        ok = client_instance.call_with_callback(service_method_name, service_request, print_service_response, timeout_ms=1000)
        time.sleep(0.5)

        # 3. Asynchronous with callback
        ok = client_instance.call_with_callback_async(service_method_name, service_request, print_service_response)
        time.sleep(0.5)

  del mirror_client

  # finalize eCAL API
  ecal_core.finalize()

if __name__ == "__main__":
  main()
