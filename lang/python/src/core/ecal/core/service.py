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
"""Module that contains the ecal server/client interface.
"""
import ecal.core.core as ecal_core

class Server(object):
  """ eCAL server
  """

  def __init__(self, service_name):
    """ initialize server

    :param service_name: the unique service name
    :type service_name:  string

    """
    # service name
    self.sname = service_name
    # server handle
    self.shandle = ecal_core.server_create(self.sname)

  def destroy(self):
    """ destroy server
    """
    return ecal_core.server_destroy(self.shandle)

  def add_method_callback(self, method_name, req_type, resp_type, callback):
    """ add method callback to server

    :param method_name: the name of method to add
    :type method_name:  string
    :param req_type:    the type of the method request (optional)
    :param resp_type:   the type of the method response (optional)
    :param callback:    the method callback function (response = f(method_name, req_type, resp_type, request))

    """
    return ecal_core.server_add_method_callback(self.shandle, method_name, req_type, resp_type, callback)

  def rem_method_callback(self, method_name):
    """ remove method callback from server

    :param method_name: the name of method to add
    :type method_name:  string

    """
    return ecal_core.server_rem_method_callback(self.shandle, method_name)


class Client(object):
  """ eCAL client
  """

  def __init__(self, service_name):
    """ initialize client

    :param service_name: the unique service name
    :type service_name:  string

    """
    # service name
    self.sname = service_name
    # server handle
    self.shandle = ecal_core.client_create(self.sname)

  def destroy(self):
    """ destroy server
    """
    return ecal_core.client_destroy(self.shandle)

  def set_hostname(self, host_name):
    """ set host name of the service

    :param host_name: the host name the service is running on
    :type host_name:  string

    """
    return ecal_core.client_set_hostname(self.shandle, host_name)

  def add_response_callback(self, callback):
    """ add response callback to client

    :param callback: the response callback function (f(service_info, response))

    """
    return ecal_core.client_add_response_callback(self.shandle, callback)

  def rem_response_callback(self):
    """ remove response callback from client
    """
    return ecal_core.client_rem_response_callback(self.shandle)

  def call_method(self, method_name, request, timeout = -1):
    """ call a method of the service

    :param method_name: the method name to call
    :type  method_name: string
    :param request:     the request argument for the method
    :type  request:     string
    :param timeout:     maximum time before operation returns (in milliseconds, -1 means infinite)
    :type  timeout:     integer

    """
    return ecal_core.client_call_method(self.shandle, method_name, request, timeout)


if __name__ == '__main__':
  """Test the service API
  """

  import time

  # initialize eCAL API
  ecal_core.initialize([], "server demo")

  # create server
  server = Server("Ping")

  # define server method request callbacks
  def ping_req_callback(method_name, req_type, resp_type, request):
    print("ping ({}) method called with {}".format(method_name, request))
    return 0, bytes("pong", "ascii")

  def echo_req_callback(method_name, req_type, resp_type, request):
    print("echo ({}) method called with {}".format(method_name, request))
    return 0, bytes("Echo: {}".format(request), "ascii")

  # add server method request callbacks
  server.add_method_callback("ping", "ping_type", "pong_type", ping_req_callback)
  server.add_method_callback("echo", "string",    "string",    echo_req_callback)

  # create client
  client = Client("Ping")

  # define client response callbacks
  def client_resp_callback(service_info, response):
    if (service_info["call_state"] == "call_state_executed"):
      print("({}) method responded {}".format(service_info["method_name"], response))
    else:
      print("server {} response failed, error '{}'".format(service_info["host_name"], service_info["error_msg"]))

  # add client response callback
  client.add_response_callback(client_resp_callback)

  # match all
  time.sleep(2.0)
  
  # call ping method
  i = 0
  while ecal_core.ok() and i < 10:
    i = i + 1
    print("ping request {} sent".format(i))
    client.call_method("ping", bytes("{}".format(i), "ascii"))
    time.sleep(1.0)

  # remove ping method
  server.rem_method_callback("ping")

  # call ping method (server method removed)
  i = 0
  while ecal_core.ok() and i < 5:
    i = i + 1
    print("ping request {} sent (server method removed)".format(i))
    client.call_method("ping", bytes("{}".format(i), "ascii"))
    time.sleep(1.0)

  # call echo method
  i = 0
  while ecal_core.ok() and i < 10:
    i = i + 1
    print("echo request {} sent".format(i))
    client.call_method("echo", b"HALLO")
    time.sleep(1.0)

  # destroy client and server
  client.destroy()
  server.destroy()

  # finalize eCAL API
  ecal_core.finalize()
