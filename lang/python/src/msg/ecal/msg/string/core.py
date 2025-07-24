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

import typing

import ecal.nanobind_core
from ..common.core import MessagePublisher, MessageSubscriber
from .serializer import Serializer as StringSerializer

class Publisher(MessagePublisher[str]):  
  def __init__(
      self,
      topic_name: str,
      config: ecal.nanobind_core.PublisherConfiguration = ecal.nanobind_core.get_publisher_configuration(), 
      event_callback: typing.Optional[object] = None) -> None:  
    super().__init__(
      StringSerializer(ecal.nanobind_core.DataTypeInformation),
      topic_name,
      config,
      event_callback
    )

class Subscriber(MessageSubscriber[str]):  
    def __init__(
        self,
        topic_name: str,
        config: ecal.nanobind_core.SubscriberConfiguration = ecal.nanobind_core.get_subscriber_configuration(), 
        event_callback: typing.Optional[object] = None) -> None:  
      super().__init__(
        StringSerializer(ecal.nanobind_core.DataTypeInformation),
        topic_name,
        config,
        event_callback
      )