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
  def __init__(self, name, topic_type="", topic_descriptor=b""):
    """ Initialize a message publisher

    :param name:             subscription name of the publisher
    :type name:              string
    :param topic_type:       optional, type of the transported payload, eg a a string, a protobuf message
    :type topic_type:        string
    :param topic_descriptor: optional, a string which can be registered with ecal to allow io
      reflection features
    :type topic_descriptor:  bytes

    """
    self.c_publisher = ecal_core.publisher(name, topic_type, topic_descriptor)

  def send(self, msg, time=-1):
    """ Send out a message

    :param msg:  Message object to send
    :param time: optional, timestamp which to send
    :type time:  int

    """
    raise NotImplementedError("Please Implement this method")

  def set_qos(self, qos):
    """ set publisher quality of service

    :param qos: 0 = default, 1 = best effort, 2 = reliable
    :type qos:  int

    """
    return self.c_publisher.set_qos(qos)

  def set_layer_mode(self, layer, mode):
    """ set send mode for specific transport layer

    :param layer: 0 = udp, 1 = shm, 2 = hdf5
    :type layer:  int
    :param mode:  0 = off, 1 = on,  2 = auto
    :type layer:  int

    """
    return self.c_publisher.set_layer_mode(layer, mode)

  def set_max_bandwidth_udp(self, bandwidth):
    """ set publisher maximum transmit bandwidth for the udp layer.

    :param bandwidth:    maximum bandwidth in bytes/s (-1 == unlimited)
    :type bandwidth:     int

    """

    return self.c_publisher.pub_set_max_bandwidth_upd(bandwidth)


class ProtoPublisher(MessagePublisher):
  """Spezialized publisher that sends out protobuf messages
  """
  def __init__(self, name, type_=None):
    if type_ is not None:
      topic_type = "proto:" + type_.DESCRIPTOR.full_name
      topic_desc = pb_helper.get_descriptor_from_type(type_)
      super(ProtoPublisher, self).__init__(name, topic_type, topic_desc)
    else:
      super(ProtoPublisher, self).__init__(name)

  def send(self, msg, time=-1):
    return self.c_publisher.send(msg.SerializeToString(), time)


class StringPublisher(MessagePublisher):
  """Spezialized publisher that sends out plain strings
  """
  def __init__(self, name):
    topic_type = "base:std::string"
    topic_desc = b""
    super(StringPublisher, self).__init__(name, topic_type, topic_desc)

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
