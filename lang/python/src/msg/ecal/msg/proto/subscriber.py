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

from ecal.core.subscriber import MessageSubscriber
import ecal.proto.helper as pb_helper

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