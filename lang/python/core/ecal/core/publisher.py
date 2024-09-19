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
"""Module that contains the ecal message publisher interface.
"""
import ecal.core.core as ecal_core
import ecal.proto.helper as pb_helper

class MessagePublisher(object):
  """Message publisher - Base class for all specialized publishers
  Classes inheriting from this class need to implement the 
  :func:`~msg_publisher.msg_publisher.send` function.
  """

  def __init__(self, topic_name, topic_type="", topic_encoding="", topic_descriptor=b""):
    """ Initialize a message publisher

    :param topic_name:       topic name of the publisher
    :type topic_name:        string
    :param topic_type:       optional, type of the transported payload, eg a a string, a protobuf message
    :type topic_type:        string
    :param topic_encoding:   optional type encoding (e.g. base, proto ..)
    :type topic_encoding:    string
    :param topic_descriptor: optional, a string which can be registered with ecal to allow io reflection features
    :type topic_descriptor:  bytes
    """
    self.c_publisher = ecal_core.publisher(topic_name, topic_type, topic_encoding, topic_descriptor)

  def send(self, msg, time=-1):
    """ Send out a message

    :param msg:  Message object to send
    :param time: optional, timestamp which to send
    :type time:  int

    """
    raise NotImplementedError("Please Implement this method")

class ProtoPublisher(MessagePublisher):
  """Spezialized publisher that sends out protobuf messages
  """
  def __init__(self, name, type_=None):
    if type_ is not None:
      topic_type = type_.DESCRIPTOR.full_name
      topic_enc  = "proto"
      topic_desc = pb_helper.get_descriptor_from_type(type_)
      super(ProtoPublisher, self).__init__(name, topic_type, topic_enc, topic_desc)
    else:
      super(ProtoPublisher, self).__init__(name)

  def send(self, msg, time=-1):
    return self.c_publisher.send(msg.SerializeToString(), time)

class BinaryPublisher(MessagePublisher):
  """Spezialized publisher that sends out binary messages
  """
  def __init__(self, name):
    topic_type = "binary"
    topic_enc = "base"
    topic_desc = b""
    super(BinaryPublisher, self).__init__(name, topic_type, topic_enc, topic_desc)

  def send(self, msg, time=-1):
    return self.c_publisher.send(msg, time)

class StringPublisher(MessagePublisher):
  """Spezialized publisher that sends out plain strings
  """
  def __init__(self, name):
    topic_type = "std::string"
    topic_enc  = "base"
    topic_desc = b""
    super(StringPublisher, self).__init__(name, topic_type, topic_enc, topic_desc)

  def send(self, msg, time=-1):
    return self.c_publisher.send(msg.encode(), time)


if __name__ == '__main__':
  """Test the publisher API
  """

  import time

  # initialize eCAL API
  ecal_core.initialize([], "publisher demo")
  
  # create publisher
  pub = StringPublisher("Hello")
  msg = "HELLO WORLD FROM PYTHON"
  
  # send messages
  i = 0
  while ecal_core.ok() and i < 1000:
    i = i + 1
    current_message = "{} {:6d}".format(msg, i)
    print("Sending: {}".format(current_message))
    pub.send(current_message)
    time.sleep(0.1)
  
  # finalize eCAL API
  ecal_core.finalize()
