# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
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

import ecal.core.core as ecal_core
import ecal.core.service as ecal_service

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # initialize eCAL API
  ecal_core.initialize(sys.argv, "py_minimal_service")
  
  # set process state
  ecal_core.set_process_state(1, 1, "I feel good")

  # create server "Ping"
  server = ecal_service.Server("DemoServer")

  # define the server method "foo" function
  def foo_req_callback(method_name, req_type, resp_type, request):
    print("'DemoService' method '{}' called with {}".format(method_name, request))
    return 0, bytes("thank you for calling foo :-)", "ascii")

  # define the server method "ping" function
  def ping_req_callback(method_name, req_type, resp_type, request):
    print("'DemoService' method '{}' called with {}".format(method_name, request))
    return 0, bytes("pong", "ascii")

  # define the server methods and connect them to the callbacks
  server.add_method_callback("foo",  "string",    "string",    foo_req_callback)
  server.add_method_callback("ping", "ping_type", "pong_type", ping_req_callback)

  # create a client for the "DemoServer" service
  client = ecal_service.Client("DemoServer")

  # define the client response callback to catch server responses
  def client_resp_callback(service_info, response):
    if (service_info["call_state"] == "call_state_executed"):
      print("'DemoService' method '{}' responded : '{}'".format(service_info["method_name"], response))
      print()
    else:
      print("server {} response failed, error : '{}'".format(service_info["host_name"], service_info["error_msg"]))
      print()

  # and add it to the client
  client.add_response_callback(client_resp_callback)

  # match all
  time.sleep(2.0)
  
  # call foo method
  i = 0
  while ecal_core.ok() and i < 20:
    i = i + 1
    request = bytes("hello foo {}".format(i), "ascii")
    print("'DemoService' method 'foo' requested with : {}".format(request))
    client.call_method("foo", request)
    time.sleep(1.0)

  # call ping method
  i = 0
  while ecal_core.ok() and i < 20:
    i = i + 1
    request = bytes("ping number {}".format(i), "ascii")
    print("'DemoService' method 'ping' requested with : {}".format(request))
    client.call_method("ping", request)
    time.sleep(1.0)

  # destroy client and server
  client.destroy()
  server.destroy()
  
  # finalize eCAL API
  ecal_core.finalize()

if __name__ == "__main__":
  main()
