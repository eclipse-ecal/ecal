# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2024 Continental Corporation
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
import time

import nanobind_core as ecal_core

  # define the client response callback to catch server responses
def client_resp_callback(service_info, response):
    if (service_info["call_state"] == "call_state_executed"):
        print("'DemoService' method '{}' responded : '{}'".format(service_info["method_name"], response))
        print()
    else:
        print("server {} response failed, error : '{}'".format(service_info["host_name"], service_info["error_msg"]))
        print()
      
def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date()))
  
  # initialize eCAL API
  ecal_core.initialize()
  
  # create a client for the "DemoService" service
  client = ecal_core.ServiceClient("DemoService")
    
  # and add it to the client
  client.add_response_callback(client_resp_callback)

  # idle and call service methods
  i = 0
  while(ecal_core.ok()):
    i = i + 1
    # call foo
    request = bytes("hello foo {}".format(i), "ascii")
    print("'DemoService' method 'foo' requested with : {}".format(request))
    client.call("foo", "request", 1234)
    time.sleep(0.5)
    # call ping
    request = bytes("ping number {}".format(i), "ascii")
    print("'DemoService' method 'ping' requested with : {}".format(request))
    client.call("ping", "request", 1234)
    time.sleep(0.5)

  # destroy client
  client.destroy()
  
  # finalize eCAL API
  ecal_core.finalize()

if __name__ == "__main__":
  main()
