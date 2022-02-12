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

"""
  @package ecal
  Python wrapper for eCAL the enhanced communication abstraction layer.

  You can use the eCAL API in a function style or OO manner.
"""

import ecal._ecal_core_py as _ecal

def initialize(args, unit_name):
  """ initialize eCAL API

  :param args:      command line argument list (sys.argv)
  :param unit_name: instance unit name
  :type unit_name:  string

  """
  return _ecal.initialize(args, unit_name)


def finalize():
  """ finalize eCAL API
  """
  return _ecal.finalize()


def getversion():
  """ get ecal version string
  """
  return _ecal.getversion()


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
  """ shutdown ecal core components
  """
  return _ecal.shutdown_core()


def enable_loopback(state):
  """ enable ecal message loopback

  :param state: switch on ecal message loop back (default 0) 
  :type state: int

  """
  return _ecal.enable_loopback(state)


def log_setlevel(level):
  """ set log level

  :param level: log level (info = 1, warning = 2, error = 3, fatal = 4)
  :type level: int

  """
  return _ecal.log_setlevel(level)


def log_setcoretime(time):
  """ log core time

  :param time: module core time in ms
  :type time: int
  
  """
  return _ecal.log_setcoretime(time)


def log_message(message):
  """ log message

  :param message: the message to log
  :type message: string

  """
  return _ecal.log_message(message)


def pub_create(topic_name, topic_type, topic_desc):
  """ create publisher

  :param topic_name: the unique topic name
  :type topic_name: string
  :param topic_type: optional type name
  :type topic_type: string
  :param topic_desc: optional type description
  :type topic_desc: bytes

  """
  topic_handle = _ecal.pub_create(topic_name, topic_type)
  pub_set_description(topic_handle, topic_desc)
  return topic_handle


def pub_destroy(topic_handle):
  """ destroy publisher
  
  :param topic_handle: the topic handle

  """
  return _ecal.pub_destroy(topic_handle)


def pub_set_description(topic_handle, description):
  """ set publisher description

  :param topic_handle: the topic handle
  :type topic_handle: string
  :param description:  the topic description
  :type description: bytes

  """
  return _ecal.pub_set_description(topic_handle, description)


def pub_set_qos_historykind(topic_handle, qpolicy, depth):
  """ set publisher quality of service historykind mode and depth

  :param topic_handle: the topic handle
  :param qpolicy: 0 = keep_last_history_qos, 1 = keep_all_history_qos
  :type qpolicy: int
  :param depth: history kind buffer depth

  """

  return _ecal.pub_set_qos_historykind(topic_handle, qpolicy, depth)


def pub_set_qos_reliability(topic_handle, qpolicy):
  """ set publisher quality of service reliability mode

  :param topic_handle: the topic handle
  :param qpolicy: 0 = best_effort_reliability_qos, 1 = reliable_reliability_qos
  :type qpolicy: int
  :param depth: history kind buffer depth
  
  """

  return _ecal.pub_set_qos_reliability(topic_handle, qpolicy)


def pub_set_layer_mode(topic_handle, layer, mode):
  """ set send mode for specific transport layer

  :param topic_handle: the topic handle
  :param layer: 0 = udp, 1 = shm, 42 = inproc
  :type layer: int
  :param mode: 0 = off, 1 = on,  2 = auto
  :type mode: int

  """

  return _ecal.pub_set_layer_mode(topic_handle, layer, mode)


def pub_set_max_bandwidth_udp(topic_handle, bandwidth):
  """ set publisher maximum transmit bandwidth for the udp layer.

  :param topic_handle: the topic handle
  :param bandwidth:    maximum bandwidth in bytes/s (-1 == unlimited)
  :type bandwidth:     int

  """

  return _ecal.pub_set_max_bandwidth_udp(topic_handle, bandwidth)


def pub_send(topic_handle, msg_payload, msg_time=-1):
  """ send publisher content

  :param topic_handle: the topic handle
  :param msg_payload:  message python string (can contain zeros)
  :type msg_payload:   bytes
  :param msg_time:     optional message time in us (default -1 == eCAL system time)
  :type msg_time:      int

  """
  return _ecal.pub_send(topic_handle, msg_payload, msg_time)


def sub_create(topic_name, topic_type):
  """ create subscriber

  :param topic_name: the unique topic name
  :type topic_name:  string
  :param topic_type: optional topic type
  :type topic_type:  string
  
  """
  return _ecal.sub_create(topic_name, topic_type)


def sub_destroy(topic_handle):
  """ destroy subscriber

  :param topic_handle: the topic handle

  """
  return _ecal.sub_destroy(topic_handle)


def sub_set_qos_historykind(topic_handle, qpolicy, depth):
  """ set subscriber quality of service historykind mode and depth

  :param topic_handle: the topic handle
  :param qpolicy:      0 = keep_last_history_qos, 1 = keep_all_history_qos
  :type qpolicy:       int
  :param depth:        history kind buffer depth
  :type depth:         int

  """

  return _ecal.sub_set_qos_historykind(topic_handle, qpolicy, depth)


def sub_set_qos_reliability(topic_handle, qpolicy):
  """ set subscriber quality of service reliability mode

  :param topic_handle: the topic handle
  :param qpolicy:      0 = best_effort_reliability_qos, 1 = reliable_reliability_qos
  :type qpolicy:       int

  """

  return _ecal.sub_set_qos_reliability(topic_handle, qpolicy)


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


def dyn_json_sub_create(topic_name):
  """ create subscriber

  :param topic_name: the unique topic name
  :type topic_name:  string

  """
  return _ecal.dyn_json_sub_create(topic_name)


def dyn_json_sub_destroy(topic_handle):
  """ destroy subscriber

  :param topic_handle: the topic handle

  """
  return _ecal.dyn_json_sub_destroy(topic_handle)


def dyn_json_sub_set_callback(topic_handle, callback):
  """ set callback function for incoming messages

  :param topic_handle: the topic handle
  :param callback:     python callback function (f(topic_name, msg, time))

  """
  return _ecal.dyn_json_sub_set_callback(topic_handle, callback)


def dyn_json_sub_rem_callback(topic_handle, callback):
  """ remove callback function for incoming messages

  :param topic_handle: the topic handle
  :param callback:     python callback function (f(topic_name, msg, time))

  """
  return _ecal.dyn_json_sub_rem_callback(topic_handle, callback)


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
  return _ecal.client_set_host_name(client_handle, host_name)


def client_add_response_callback(client_handle, callback):
  """ add response callback to client

  :param client_handle: the client handle
  :param callback:      the response callback function (f(service_info, response))

  """
  return _ecal.client_add_response_callback(client_handle, callback)


def client_rem_method_callback(client_handle):
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


def mon_setexclfilter(filter_):
  """ Set topics filter blacklist regular expression

  :param filter_: filter string as regular expression
  :type filter_: string

  """
  return _ecal.mon_setexclfilter(filter_)


def mon_setinclfilter(filter_):
  """ Set topics filter whitelist regular expression

  :param filter_: filter string as regular expression
  :type filter_: string
  
  """
  return _ecal.mon_setinclfilter(filter_)


def mon_setfilterstate(state_):
  """ Switch eCAL monitoring filter on/off

  :param state_: filter state on / off

  """
  return _ecal.mon_setfilterstate(state_)


def mon_monitoring():
  """ get all host, process and topic information as dictionary
  """
  return _ecal.mon_monitoring()


def mon_logging():
  """ get list of ecal log messages
  """
  return _ecal.mon_logging()


def mon_pubmonitoring(state_, name_):
  """ activate an eCAL internal publisher for monitoring info

  :param state_: publisher state on / off
  :param name_:  topic name for the publisher
  :type name_:   string

  """
  return _ecal.mon_pubmonitoring(state_, name_)


def mon_publogging(state_, name_):
  """ activate an eCAL internal publisher for logging info

  :param state_: publisher state on / off
  :param name_:  topic name for the publisher
  :type name_:   string  

  """
  return _ecal.mon_publogging(state_, name_)


class publisher(object):
  """ eCAL publisher """

  def __init__(self, topic_name, topic_type="", topic_desc=b""):
    """ initialize publisher

    :param topic_name: the unique topic name
    :type topic_name:  string
    :param topic_type: optional type name
    :type topic_type:  string
    :param topic_desc: optional type description
    :type topic_desc:  bytes

    """
    # topic name
    self.tname = topic_name
    # topic type
    self.ttype = topic_type
    # topic description
    self.tdesc = topic_desc
    # topic handle
    self.thandle = pub_create(self.tname, self.ttype, self.tdesc)
    
  def destroy(self):
    """ destroy publisher
    """
    return pub_destroy(self.thandle)

  def set_qos_historykind(self, qpolicy, depth):
    """ set quality of service historykind mode and depth

    :param qpolicy: 0 = keep_last_history_qos, 1 = keep_all_history_qos
    :param depth:   history kind buffer depth

    """

    return pub_set_qos_historykind(self.thandle, qpolicy, depth)

  def set_qos_reliability(self, qpolicy):
    """ set quality of service reliability mode

    :param qpolicy: 0 = best_effort_reliability_qos, 1 = reliable_reliability_qos

    """

    return pub_set_qos_reliability(self.thandle, qpolicy)

  def set_layer_mode(self, layer, mode):
    """ set send mode for specific transport layer

    :param layer: 0 = udp, 1 = shm, 42 = inproc
    :type layer:  int
    :param mode:  0 = off, 1 = on,  2 = auto
    :type layer:  int
    
    """
    return _ecal.pub_set_layer_mode(self.thandle, layer, mode)

  def set_max_bandwidth_udp(self, bandwidth):
    """ set publisher maximum transmit bandwidth for the udp layer.

    :param bandwidth: maximum bandwidth in bytes/s (-1 == unlimited)
    :type bandwidth:  int

    """

    return _ecal.pub_set_max_bandwidth_udp(self.thandle, bandwidth)

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

  def __init__(self, topic_name, topic_type=""):
    """ initialize subscriber

    :param topic_name: the unique topic name
    :type topic_name:  string
    :param topic_type: optional topic type
    :type topic_type:  string

    """
    # topic name
    self.tname = topic_name
    # topic type
    self.ttype = topic_type
    # topic handle
    self.thandle = sub_create(self.tname, self.ttype)
    
  def destroy(self):
    """ destroy subscriber
    """
    return sub_destroy(self.thandle)

  def set_qos_historykind(self, qpolicy, depth):
    """ set quality of service historykind mode and depth

    :param qpolicy: 0 = keep_last_history_qos, 1 = keep_all_history_qos
    :type qpolicy:  int
    :param depth:   history kind buffer depth
    :type depth:    int

    """

    return sub_set_qos_historykind(self.thandle, qpolicy, depth)

  def set_qos_reliability(self, qpolicy):
    """ set quality of service reliability mode

    :param qpolicy: 0 = best_effort_reliability_qos, 1 = reliable_reliability_qos
    :type qpolicy:  int

    """

    return sub_set_qos_reliability(self.thandle, qpolicy)

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


class subscriberDynJSON(object):
  """ eCAL Protobuf dynamic JSON subscriber
  """

  def __init__(self, topic_name):
    """ initialize subscriber

    :param topic_name: the unique topic name
    :type topic_name:  string

    """
    # topic name
    self.tname = topic_name
    # topic handle
    self.thandle = dyn_json_sub_create(self.tname)
    
  def destroy(self):
    """ destroy subscriber
    """
    return dyn_json_sub_destroy(self.thandle)

  def set_callback(self, callback):
    """ set callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    return dyn_json_sub_set_callback(self.thandle, callback)

  def rem_callback(self, callback):
    """ remove callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    return dyn_json_sub_rem_callback(self.thandle, callback)
