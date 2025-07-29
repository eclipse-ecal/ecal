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
  ecal_core.initialize("py_minimal_service_server")
  
  # set process state
  ecal_core.set_process_state(1, 1, "I feel good")

  # create a server for the "DemoService" service
  server = ecal_service.Server("DemoService")

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

  # idle
  while(ecal_core.ok()):
    time.sleep(1.0)

  # destroy server
  server.destroy()
  
  # finalize eCAL API
  ecal_core.finalize()

if __name__ == "__main__":
  main()
