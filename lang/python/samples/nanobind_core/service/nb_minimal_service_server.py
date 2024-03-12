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

import ecal.nanobind_core as ecal_core

# define the server method "ping" function
def echo_req_callback(method_name, req_type, resp_type, request):
    print("Method called'{}' called with {}".format(method_name, request))
    return 0, request

# define the server method "ping" function
def reverse_req_callback(method_name, req_type, resp_type, request):
    print("Method called'{}' called with {}".format(method_name, request))
    response = request[::-1] #reverse the request
    return 0, response

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date()))
  
  # initialize eCAL API
  ecal_core.initialize()
  ecal_core.set_unitname("py_minimal_service_server")
  
  # create a server for the "DemoService" service
  server = ecal_core.ServiceServer("service1")

  # define the server methods and connect them to the callbacks
  server.add_method_callback("echo", "", "", echo_req_callback)
  server.add_method_callback("reverse", "", "", reverse_req_callback)

  # idle
  while(ecal_core.ok()):
    time.sleep(0.1)

  # destroy server
  server.destroy()
  
  # finalize eCAL API
  server.finalize()

if __name__ == "__main__":
  main()
