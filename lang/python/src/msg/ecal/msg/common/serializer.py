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

class SerializationError(Exception):
    """Exception raised (de)serializing an object"""
    pass

class UnsupportedDatatypeError(Exception):
    """This exception is raised if a datatype cannot be deserialized by the given deserializer"""
    pass

class DataTypeInfo(typing.Protocol):
    def __init__(self, name: str = "", encoding: str = "", descriptor: bytes = b"") -> None:
        self.name = name
        self.encoding = encoding
        self.descriptor = descriptor

    descriptor: bytes
    encoding: str
    name: str

T = typing.TypeVar("T")

class Serializer(typing.Protocol[T]):
    """
    Protocol for a class that can serialize objects of type T to bytes
    """

    def get_data_type_information(self) -> DataTypeInfo:
        """
        DataTypeInfo associated with the instance of this Serializer.
        """
        ...

    def serialize(self, message: T) -> bytes: 
        """
        Serialize the given message, returning a bytes object.
        """
        ...    


class Deserializer(typing.Protocol[T]):    
    """
    Protocol for a class that can deserialize bytes to objects of type T
    """

    def get_data_type_information(self) -> DataTypeInfo:
        """
        DataTypeInfo associated with the instance of this Serializer.
        """
        ...

    def accepts_data_with_type(self, data_type_info: DataTypeInfo) -> bool:
        """
        Checks whether incoming data with the given data_type_info can be deserialized by this Serializer
        """
        ...  
    
    def deserialize(self, buffer: bytes, data_type_info: DataTypeInfo) -> T: 
        """
        Deserialize the data provided in buffer. 
        May raise `SerializationError` if anything goes wrong during deserialization, 
        such as the Datatypeinfo doesn't match.
        """
        ...