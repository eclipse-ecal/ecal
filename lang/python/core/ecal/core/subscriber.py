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
"""Module that contains the ecal message subscriber interface.
"""
import ecal.core.core as ecal_core
import ecal.proto.helper as pb_helper

class MessageSubscriber(object):
  """Message subscriber - Base class for all specialized subscribers
  Classes inheriting from this class need to implement the 
  :func:`~subscriber.MessageSubscriber.receive`, 
  :func:`~subscriber.MessageSubscriber.set_callback` and
  :func:`~subscriber.MessageSubscriber.rem_callback` functions.
  """

  def __init__(self, topic_name, topic_type="", topic_encoding="", topic_descriptor=b""):
    """ Initialize a message subscriber

    :param topic_name:       topic name of the subscriber
    :type topic_name:        string
    :param topic_type:       optional, type of the transported payload, eg a a string, a protobuf message
    :type topic_type:        string
    :param topic_encoding:   optional type encoding (e.g. base, proto ..)
    :type topic_encoding:    string
    :param topic_descriptor: optional, a string which can be registered with ecal to allow io reflection features
    :type topic_descriptor:  bytes
    """
    self.c_subscriber = ecal_core.subscriber(topic_name, topic_type, topic_encoding, topic_descriptor)

  def receive(self, timeout=0):
    """ receive subscriber content with timeout

    :param timeout: receive timeout in ms
    :type timeout:  int

    """
    raise NotImplementedError("Please Implement this method")

  def set_callback(self, callback):
    """ set callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    raise NotImplementedError("Please Implement this method")

  def rem_callback(self, callback):
    """ remove callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    raise NotImplementedError("Please Implement this method")
    

class ProtoSubscriber(MessageSubscriber):
  """Spezialized subscriber that subscribes to protobuf messages
  """
  def __init__(self, name, type_protobuf):
    topic_type = type_protobuf.DESCRIPTOR.full_name
    topic_enc  = "proto"
    topic_desc = pb_helper.get_descriptor_from_type(type_protobuf)
    super(ProtoSubscriber, self).__init__(name, topic_type, topic_enc, topic_desc)
    self.protobuf_type = type_protobuf
    self.callback = None

  def receive(self, timeout=0):
    """ receive subscriber content with timeout

    :param timeout: receive timeout in ms
    :type timeout:  int

    """
    ret, msg, time = self.c_subscriber.receive(timeout)
    proto_message = self.protobuf_type()
    if ret > 0:
      # parse message content from 'msg'
      proto_message.ParseFromString(msg)
    return ret, proto_message, time

  def set_callback(self, callback):
    """ set callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    self.callback = callback
    self.c_subscriber.set_callback(self._on_receive)

  def rem_callback(self, callback):
    """ remove callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    self.c_subscriber.rem_callback(self._on_receive)
    self.callback = None

  def _on_receive(self, topic_name, msg, time):
    proto_message = self.protobuf_type()
    proto_message.ParseFromString(msg)
    self.callback(topic_name, proto_message, time)


class BinarySubscriber(MessageSubscriber):
  """Specialized subscriber that subscribes to binary messages
  """
  def __init__(self, name):
    topic_type = "binary"
    topic_enc = "base"
    topic_desc = b""
    super(BinarySubscriber, self).__init__(name, topic_type, topic_enc, topic_desc)
    self.callback = None

  def receive(self, timeout=0):
    """ receive subscriber content with timeout
    
    :param timeout: receive timeout in ms
    
    """
    ret, msg, time = self.c_subscriber.receive(timeout)
    if ret > 0:
      msg = msg
    else:
      msg = b""
    return ret, msg, time
  
  def set_callback(self, callback):
    """ set callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    self.callback = callback
    self.c_subscriber.set_callback(self._on_receive)

  def rem_callback(self, callback):
    """ remove callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    self.c_subscriber.rem_callback(self._on_receive)
    self.callback = None

  def _on_receive(self, topic_name, msg, time):
    self.callback(topic_name, msg, time)


class StringSubscriber(MessageSubscriber):
  """Spezialized publisher subscribes to plain strings
  """
  def __init__(self, name):
    topic_type = "std::string"
    topic_enc  = "base"
    topic_desc = b""
    super(StringSubscriber, self).__init__(name, topic_type, topic_enc, topic_desc)
    self.callback = None

  def receive(self, timeout=0):
    """ receive subscriber content with timeout

    :param timeout: receive timeout in ms

    """
    ret, msg, time = self.c_subscriber.receive(timeout)
    if ret > 0:
      msg = msg.decode()
    else:
      msg = ""
    return ret, msg, time

  def set_callback(self, callback):
    """ set callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    self.callback = callback
    self.c_subscriber.set_callback(self._on_receive)

  def rem_callback(self, callback):
    """ remove callback function for incoming messages

    :param callback: python callback function (f(topic_name, msg, time))

    """
    self.c_subscriber.rem_callback(self._on_receive)
    self.callback = None

  def _on_receive(self, topic_name, msg, time):
    self.callback(topic_name, msg.decode(), time)    


if __name__ == '__main__':
  """Test the subscriber API
  """

  import time

  # initialize eCAL API
  ecal_core.initialize([], "subscriber demo")
  
  # create subscriber
  sub = StringSubscriber("Hello")

  # let's match all
  time.sleep(1.0)
  
  print("\n\n---Enter polling mode---\n\n")

  # use the blocking receive variant
  i = 0
  while ecal_core.ok() and i < 50:
    i = i + 1
    ret, msg, t = sub.receive(200)
    if ret > 0:
      print("Blocking receive: '{}'".format(msg))
    else:
      print("Blocking receive timeout ..")

  print("\n\n---Enter callback mode---\n\n")

  # receive callback
  def callback(topic_name, msg, time):
    print("Callback receive: '{}'".format(msg))

  # use the callback variant
  sub.set_callback(callback)
  
  # idle for 5 seconds
  i = 0
  while ecal_core.ok() and i < 50:
    i = i + 1
    time.sleep(0.1)

  # finalize eCAL API
  ecal_core.finalize()
