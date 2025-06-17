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

from ecal.core.publisher import MessagePublisher
import ecal.proto.helper as pb_helper

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