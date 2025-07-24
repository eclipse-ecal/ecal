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

import pytest
import time
from typing import Tuple
from ecal.msg.string.serializer import Serializer
from ecal.msg.common.serializer import DataTypeInfo

correct_datatype_info = DataTypeInfo(name = "string", encoding = "utf-8")
incorrect_datatype_info = DataTypeInfo(name = "other", encoding = "utf-8")

def test_datatype_information():
    serializer = Serializer(DataTypeInfo)
    assert serializer.get_data_type_information == correct_datatype_info

def test_accepts_datatype_information():
    serializer = Serializer(DataTypeInfo)   
    assert serializer.accepts_data_with_type(correct_datatype_info)
    assert not serializer.accepts_data_with_type(incorrect_datatype_info)

def test_serialize_string():
    serializer = Serializer(DataTypeInfo)

    assert serializer.serialize("") == b""
    assert serializer.serialize("abc") == b"abc"


def test_serialize_string():
    deserializer = Serializer(DataTypeInfo)

    assert deserializer.deserialize(b"") == ""
    assert deserializer.deserialize(b"abc") == "abc"

    #see that decoding something which cannot be decoded throws an Deserialization error