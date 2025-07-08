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
from ..common.serializer import SerializationError, DataTypeInfo

class Serializer:
    def __init__(self, data_type_type : typing.Type[DataTypeInfo]):
        self._data_type_type = data_type_type

    def get_data_type_information(self) -> DataTypeInfo:
        return self._data_type_type(
           name = "string",
           encoding = "utf-8"
        )
   
    def accepts_data_with_type(self, data_type_info: DataTypeInfo) -> bool:
        return self.get_data_type_information() == data_type_info
    
    def serialize(self, message: str) -> bytes: 
        return message.encode()
    
    def deserialize(self, buffer: bytes, data_type_info: DataTypeInfo) -> str:
        try:
            return buffer.decode()
        except UnicodeDecodeError as e:
            raise SerializationError(e.reason)

