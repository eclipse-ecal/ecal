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
 
import ecal.nanobind_core as ecal_core

EMPTY_STRING = ""
EMPTY_BYTES = b""
MY_NAME = "name"
MY_ENCODING = "encoding"
MY_DESCRIPTOR = b"descriptor"

class TestDataTypeInformation:
  def test_constructor_empty(self):
    default_datatype_info = ecal_core.DataTypeInformation()
    assert default_datatype_info.name == EMPTY_STRING
    assert default_datatype_info.encoding == EMPTY_STRING
    assert default_datatype_info.descriptor == EMPTY_BYTES
  
  def test_constructor_name(self):
    default_datatype_info = ecal_core.DataTypeInformation(name=MY_NAME)
    assert default_datatype_info.name == MY_NAME
    assert default_datatype_info.encoding == EMPTY_STRING
    assert default_datatype_info.descriptor == EMPTY_BYTES
  
  def test_constructor_encoding(self):
    default_datatype_info = ecal_core.DataTypeInformation(encoding=MY_ENCODING)
    assert default_datatype_info.name == EMPTY_STRING
    assert default_datatype_info.encoding == MY_ENCODING
    assert default_datatype_info.descriptor == EMPTY_BYTES
  
  def test_constructor_type(self):
    default_datatype_info = ecal_core.DataTypeInformation(descriptor=MY_DESCRIPTOR)
    assert default_datatype_info.name == EMPTY_STRING
    assert default_datatype_info.encoding == EMPTY_STRING
    assert default_datatype_info.descriptor == MY_DESCRIPTOR
  
  def test_constructor_full(self):
    default_datatype_info = ecal_core.DataTypeInformation(
      name=MY_NAME, encoding=MY_ENCODING, descriptor=MY_DESCRIPTOR
    )
    assert default_datatype_info.name == MY_NAME
    assert default_datatype_info.encoding == MY_ENCODING
    assert default_datatype_info.descriptor == MY_DESCRIPTOR