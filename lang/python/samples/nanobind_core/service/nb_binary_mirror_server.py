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
from typing import Tuple

import ecal.nanobind_core as ecal_core

def print_data(method_name : str, request : bytes, response : bytes):
  print("Method   : '{}' called".format(method_name))
  print("Requests : {}".format(request))
  print("Response : {}".format(response))
  print()

# define the server method "ping" function
def echo_req_callback(
    method_information : ecal_core.ServiceMethodInformation,
    request : bytes) -> Tuple[int, bytes]:
  response = request  
  print_data(method_information.method_name, request, response)
  return 0, response

# define the server method "ping" function
def reverse_req_callback(
    method_information : ecal_core.ServiceMethodInformation,
    request : bytes) -> Tuple[int, bytes]:
  response = request[::-1] #reverse the request
  print_data(method_information.method_name, request, response)
  return 0, response

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date_string()))
  
  # initialize eCAL API
  ecal_core.initialize("mirror server python")
  
  # create a server for the "DemoService" service
  server = ecal_core.ServiceServer("mirror")

  # define service methods and add them to the callbacks
  echo_method_info = ecal_core.ServiceMethodInformation()
  echo_method_info.method_name = "echo"
  server.set_method_callback(echo_method_info, echo_req_callback)

  reverse_method_info = ecal_core.ServiceMethodInformation()
  reverse_method_info.method_name = "reverse"
  server.set_method_callback(reverse_method_info, reverse_req_callback)
  
  # we can retrieve the services name / id
  print(server.get_service_name())
  print(server.get_service_id())

  # we can check if clients have connected to this service
  print(server.is_connected())


  # idle
  while(ecal_core.ok()):
    time.sleep(0.1)

  # destroy server
  server.destroy()
  
  # finalize eCAL API
  server.finalize()

if __name__ == "__main__":
  main()
