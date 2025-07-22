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
  
from collections import defaultdict
import typing
from ecal.msg.common.serializer import SerializationError, DataTypeInfo
from . import helper as pb_helper
from google.protobuf.message import Message

T = typing.TypeVar("T", bound=Message)

class Serializer(typing.Generic[T]):
    def __init__(self, protobuf_message_type: typing.Type[T], data_type_type: typing.Type[DataTypeInfo]):
        self._protobuf_message_type = protobuf_message_type
        self._data_type_type = data_type_type

    def get_data_type_information(self) -> DataTypeInfo:
        return self._data_type_type(
           name = self._protobuf_message_type.DESCRIPTOR.full_name,
           encoding = "proto",
           descriptor = pb_helper.get_descriptor_from_type(self._protobuf_message_type)
        )
   
    def accepts_data_with_type(self, data_type_info: DataTypeInfo) -> bool:
        return self.get_data_type_information() == data_type_info
    
    def serialize(self, message: T) -> bytes: 
        return message.SerializeToString()
    
    def deserialize(self, buffer: bytes, data_type_info: DataTypeInfo) -> T:
        try:
            proto_message = self._protobuf_message_type()
            proto_message.ParseFromString(buffer)
            return proto_message
        except Exception as e:
            raise SerializationError(e.reason)
        

class DynamicSerializer:
    class KeyAwareDefaultDict(defaultdict):
        def __missing__(self, key):
            # Call the factory function with the key
            self[key] = self.default_factory(key)
            return self[key]

    def __init__(self, data_type_type : typing.Type[DataTypeInfo]):
        self._data_type_type = data_type_type
        self._proto_message_map = self.KeyAwareDefaultDict(
            lambda key: pb_helper.get_type_from_descriptor(key.name, key.descriptor)
        )

    def get_data_type_information(self) -> DataTypeInfo:
        return self._data_type_type(
            name = "*",
            encoding = "proto",
            descriptor = b"*"
        )
   
    def accepts_data_with_type(self, data_type_info: DataTypeInfo) -> bool:
        # For dynamic serializers, we assume they accept any data with proto encoding
        return self.get_data_type_information().encoding == data_type_info.encoding

    def deserialize(self, buffer: bytes, data_type_info: DataTypeInfo) -> str:
      try:
        proto_message_class = self._proto_message_map[data_type_info]
        proto_message = proto_message_class()
        proto_message.ParseFromString(buffer)
        return proto_message
      except Exception as e:
        raise SerializationError(e.reason)


