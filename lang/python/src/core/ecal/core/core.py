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

"""
  @package ecal
  Python wrapper for eCAL the enhanced communication abstraction layer.

  You can use the eCAL API in a function style or OO manner.
"""

import ecal._ecal_core_py as _ecal

INIT_PUBLISHER  = 0x001
INIT_SUBSCRIBER = 0x002
INIT_SERVICE    = 0x004
INIT_MONITORING = 0x008
INIT_LOGGING    = 0x010
INIT_TIMESYNC   = 0x020
INIT_ALL        = INIT_PUBLISHER | INIT_SUBSCRIBER | INIT_SERVICE | INIT_MONITORING | INIT_LOGGING | INIT_TIMESYNC;
INIT_DEFAULT    = INIT_PUBLISHER | INIT_SUBSCRIBER | INIT_SERVICE | INIT_LOGGING | INIT_TIMESYNC;

def initialize(unit_name, components = None):
  """ initialize eCAL API

  :param unit_name: instance unit name
  :type unit_name:  string
  :param components: components to initialize
  :type components: int

  """
  if (components is not None):
    return _ecal.initialize_components(unit_name, components)
  else:
    return _ecal.initialize(unit_name)


def finalize():
  """ finalize eCAL API
  """
  return _ecal.finalize()


def is_initialized():
  """ get if ecal has been initialized
  """
  return _ecal.is_initialized()


def getversion():
  """ get ecal version string
  """
  return _ecal.getversion()


def getversion_components():
  """ get ecal version as major, minor, patch tuple
  """
  return _ecal.getversion_components()


def getdate():
  """ get ecal date string
  """
  return _ecal.getdate()


def getmicroseconds():
  """ get ecal microsecond timestamp
  """
  return _ecal.getmicroseconds()


def set_process_state(severity, level, info):
  '''
  set the process state
  
  :param severity: severity (unknown = 0, healthy = 1, warning = 2, critical = 3, failed = 4)
  :type severity: int
  :param level:    severity level (level1 = 1, level2 = 2, level3 = 3, level4 = 4, level5 = 5)
  :type level: int
  :param info:  info message
  :type info: string
  '''
  return _ecal.set_process_state(severity, level, info)


def ok():
  """ returns ecal process state
  """
  return _ecal.ok()


def shutdown_process_uname(unit_name):
  """ shutdown ecal user processes with specified unit name
  
  :param unit_name: unit name of the process to shut down
  :type severity: string 
  """
  return _ecal.shutdown_process_uname(unit_name)


def shutdown_process_id(process_id):
  """ shutdown ecal user processes with specified process id
  
  :param process_id: process ID of the process to shut down
  :type severity: int  
  """
  return _ecal.shutdown_process_id(process_id)


def shutdown_processes():
  """ shutdown all ecal user processes
  """
  return _ecal.shutdown_processes()


def shutdown_core():
  """ shutdown ecal core components (deprecated)
  """
  return _ecal.shutdown_processes()


def log_setlevel(level):
  """ set log level

  :param level: log level (info = 1, warning = 2, error = 3, fatal = 4)
  :type level: int

  """
  return _ecal.log_setlevel(level)


def log_message(message):
  """ log message

  :param message: the message to log
  :type message: string

  """
  return _ecal.log_message(message)


def pub_create(topic_name, topic_type, topic_enc, topic_desc):
  """ create publisher

  :param topic_name: the unique topic name
  :type topic_name: string
  :param topic_type: optional type name
  :type topic_type: string
  :param topic_enc: optional type encoding
  :type topic_enc: string
  :param topic_desc: optional type description
  :type topic_desc: bytes

  """
  topic_handle = _ecal.pub_create(topic_name, topic_type, topic_enc, topic_desc)
  return topic_handle


def pub_destroy(topic_handle):
  """ destroy publisher
  
  :param topic_handle: the topic handle

  """
  return _ecal.pub_destroy(topic_handle)


def pub_send(topic_handle, msg_payload, msg_time=-1):
  """ send publisher content

  :param topic_handle: the topic handle
  :param msg_payload:  message python string (can contain zeros)
  :type msg_payload:   bytes
  :param msg_time:     optional message time in us (default -1 == eCAL system time)
  :type msg_time:      int

  """
  return _ecal.pub_send(topic_handle, msg_payload, msg_time)


def sub_create(topic_name, topic_type, topic_enc, topic_desc):
  """ create subscriber

  :param topic_name: the unique topic name
  :type topic_name: string
  :param topic_type: optional type name
  :type topic_type: string
  :param topic_enc: optional type encoding
  :type topic_enc: string
  :param topic_desc: optional type description
  :type topic_desc: bytes

  """
  topic_handle = _ecal.sub_create(topic_name, topic_type, topic_enc, topic_desc)
  return topic_handle


def sub_destroy(topic_handle):
  """ destroy subscriber

  :param topic_handle: the topic handle

  """
  return _ecal.sub_destroy(topic_handle)


def sub_receive(topic_handle, timeout=0):
  """ receive subscriber content with timeout

  :param topic_handle: the topic handle
  :param timeout:      receive timeout in ms
  :type timeout:       int

  """
  return _ecal.sub_receive(topic_handle, timeout)


def sub_set_callback(topic_handle, callback):
  """ set callback function for incoming messages

  :param topic_handle: the topic handle
  :param callback:     python callback function (f(topic_name, msg, time))

  """
  return _ecal.sub_set_callback(topic_handle, callback)


def sub_rem_callback(topic_handle, callback):
  """ remove callback function for incoming messages

  :param topic_handle: the topic handle
  :param callback:     python callback function (f(topic_name, msg, time))

  """
  return _ecal.sub_rem_callback(topic_handle, callback)


def server_create(service_name):
  """ create server

  :param service_name: the unique service name
  :type service_name:  string

  """
  return _ecal.server_create(service_name)


def server_destroy(server_handle):
  """ destroy server

  :param server_handle: the server handle

  """
  return _ecal.server_destroy(server_handle)


def server_add_method_callback(server_handle, method_name, req_type, resp_type, callback):
  """ add method callback to server

  :param server_handle: the server handle
  :param method_name:   the name of method to add
  :param method_name:   string
  :param req_type:      the type of the method request (optional)
  :param resp_type:     the type of the method response (optional)
  :param callback:      the method callback function (response = f(method_name, req_type, resp_type, request))

  """
  return _ecal.server_add_method_callback(server_handle, method_name, req_type, resp_type, callback)


def server_rem_method_callback(server_handle, method_name):
  """ remove method callback from server

  :param server_handle: the server handle
  :param method_name:   the name of method to add
  :param method_name:   string

  """
  return _ecal.server_rem_method_callback(server_handle, method_name)


def client_create(service_name):
  """ create client

  :param service_name: the unique service name
  :type service_name:  string

  """
  return _ecal.client_create(service_name)


def client_destroy(client_handle):
  """ destroy client

  :param client_handle: the client handle

  """
  return _ecal.client_destroy(client_handle)


def client_set_hostname(client_handle, host_name):
  """ set host name of the service

  :param client_handle: the client handle
  :param host_name:     the host name the service is running on
  :type host_name:      string

  """
  return _ecal.client_set_hostname(client_handle, host_name)


def client_add_response_callback(client_handle, callback):
  """ add response callback to client

  :param client_handle: the client handle
  :param callback:      the response callback function (f(service_info, response))

  """
  return _ecal.client_add_response_callback(client_handle, callback)


def client_rem_response_callback(client_handle):
  """ remove response callback from client

  :param client_handle: the client handle

  """
  return _ecal.client_rem_response_callback(client_handle)


def client_call_method(client_handle, method_name, request, timeout = -1):
  """ call a method of the service

  :param client_handle: the client handle
  :param method_name:   the method name to call
  :type  method_name:   string
  :param request:       the request argument for the method
  :type  request:       bytes
  :param timeout:       maximum time before operation returns (in milliseconds, -1 means infinite)
  :type  timeout:       integer

  """
  return _ecal.client_call_method(client_handle, method_name, request, timeout)


def mon_initialize():
  """ initialize eCAL monitoring API
  """
  return _ecal.mon_initialize()


def mon_finalize():
  """ finalize eCAL monitoring API
  """
  return _ecal.mon_finalize()


def mon_monitoring():
  """ get all host, process and topic information as dictionary
  """
  return _ecal.mon_monitoring()


def mon_logging():
  """ get list of ecal log messages
  """
  return _ecal.mon_logging()


class publisher(object):
  """ eCAL publisher """

  def __init__(self, topic_name, topic_type="", topic_enc="", topic_desc=b""):
    """ initialize publisher

    :param topic_name: the unique topic name
    :type topic_name: string
    :param topic_type: optional type name
    :type topic_type: string
    :param topic_enc: optional type encoding
    :type topic_enc: string
    :param topic_desc: optional type description
    :type topic_desc: bytes

    """
    # topic name
    self.tname = topic_name
    # topic type
    self.ttype = topic_type
    # topic description
    self.tenc = topic_enc
    # topic description
    self.tdesc = topic_desc
    # topic handle
    self.thandle = pub_create(self.tname, self.ttype, self.tenc, self.tdesc)
    
  def destroy(self):
    """ destroy publisher
    """
    return pub_destroy(self.thandle)

  def send(self, msg_payload, msg_time=-1):
    """ send publisher content

    :param msg_payload: message python string (can contain zeros)
    :type msg_payload:  bytes
    :param msg_time:    optional message time in us (default -1 == eCAL system time)
    :type msg_time:     int

    """
    return pub_send(self.thandle, msg_payload, msg_time)


class subscriber(object):
  """ eCAL subscriber
  """

  def __init__(self, topic_name, topic_type="", topic_enc="", topic_desc=b""):
    """ initialize subscriber

    :param topic_name: the unique topic name
    :type topic_name: string
    :param topic_type: optional type name
    :type topic_type: string
    :param topic_enc: optional type encoding
    :type topic_enc: string
    :param topic_desc: optional type description
    :type topic_desc: bytes

    """
    # topic name
    self.tname = topic_name
    # topic type
    self.ttype = topic_type
    # topic description
    self.tenc = topic_enc
    # topic description
    self.tdesc = topic_desc
    # topic handle
    self.thandle = sub_create(self.tname, self.ttype, self.tenc, self.tdesc)
    
  def destroy(self):
    """ destroy subscriber
    """
    return sub_destroy(self.thandle)

  def receive(self, timeout=0):
    """ receive subscriber content with timeout

    :param timeout: receive timeout in ms
    :type timeout:  int

    """
    return sub_receive(self.thandle, timeout)

  def set_callback(self, callback):
    """ set callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    return sub_set_callback(self.thandle, callback)

  def rem_callback(self, callback):
    """ remove callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    return sub_rem_callback(self.thandle, callback)
