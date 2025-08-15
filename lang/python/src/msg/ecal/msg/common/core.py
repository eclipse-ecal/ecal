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

from dataclasses import dataclass
import typing
from .serializer import Serializer, Deserializer, SerializationError
import ecal.nanobind_core

T = typing.TypeVar("T")

@dataclass
class ReceiveCallbackData(typing.Generic[T]):
    message: T
    send_timestamp: int
    send_clock: int

DataCallbackType = typing.Callable[[ecal.nanobind_core.TopicId, ReceiveCallbackData[T]], None]
DeserializationErrorCallbackType = typing.Callable[[str, ecal.nanobind_core.TopicId, ecal.nanobind_core.DataTypeInformation, ecal.nanobind_core.ReceiveCallbackData], None]

class MessagePublisher(typing.Generic[T]):
  def __init__(
      self, 
      serializer : Serializer[T],
      topic_name: str, 
      config : ecal.nanobind_core.PublisherConfiguration = ecal.nanobind_core.get_publisher_configuration(), 
      event_callback: typing.Optional[object] = None) -> None:   
    self._serializer = serializer
    self._publisher : ecal.nanobind_core.Publisher = ecal.nanobind_core.Publisher(topic_name, serializer.get_data_type_information(), config, event_callback)  
  
  def get_data_type_information(self) -> ecal.nanobind_core.DataTypeInformation:
    return self._publisher.get_data_type_information()
    
  def get_subscriber_count(self) -> int:
    return self._publisher.get_subscriber_count()
    
  def get_topic_id(self) -> ecal.nanobind_core.TopicId:
    return self._publisher.get_topic_id()
    
  def send(self, payload: T, time: int = -1) -> bool:
   return self._publisher.send(self._serializer.serialize(payload), time)

class MessageSubscriber(typing.Generic[T]):
  def __init__(
        self, 
        serializer : Deserializer[T],        
        topic_name: str, 
        config: ecal.nanobind_core.SubscriberConfiguration = ecal.nanobind_core.get_subscriber_configuration(),
        event_callback: typing.Optional[object] = None,
        ) -> None:
    self._deserializer = serializer
    self._subscriber : ecal.nanobind_core.Subscriber = ecal.nanobind_core.Subscriber(topic_name, serializer.get_data_type_information(), config, event_callback)  


  def get_data_type_information(self) -> ecal.nanobind_core.DataTypeInformation:
    return self._subscriber.get_data_type_information()         

  def get_publisher_count(self) -> int:
    return self._subscriber.get_publisher_count()         
         
  def get_topic_id(self) -> ecal.nanobind_core.TopicId:
    return self._subscriber.get_topic_id()
  
  def get_topic_name(self) -> str:
    return self._subscriber.get_topic_name()

  def remove_receive_callback(self) -> None:
    self._subscriber.remove_receive_callback()
  
  def set_receive_callback(self, 
                           data_callback: typing.Optional[DataCallbackType[T]], 
                           error_callback: typing.Optional[DeserializationErrorCallbackType] = None) -> None:
    
    """
    Sets the internal receive callback, which deserializes the message and dispatches it
    to the appropriate user-provided callback.
    """
    
    deserializer = self._deserializer

    def internal_receive_callback(publisher_id, data_type_info, data):
      if data_callback is None and error_callback is None:
        return

      try:
        msg = deserializer.deserialize(data.buffer, data_type_info)
        if data_callback is not None:
          data_callback(publisher_id, ReceiveCallbackData(message=msg, send_timestamp=data.send_timestamp, send_clock=data.send_clock))
      except SerializationError as error:
        if error_callback is not None:
          error_callback(str(error), publisher_id, data_type_info, data)
      except Exception as error:
        print(f"Error occured during callback execution: {error}")

    self._subscriber.set_receive_callback(internal_receive_callback)