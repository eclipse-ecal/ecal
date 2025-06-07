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
DataTypeInformation      = ecal_core.DataTypeInformation
ServiceMethodInformation = ecal_core.ServiceMethodInformation

EMPTY_STRING = ""
METHOD_NAME = "echo"
REQUEST_TYPE = DataTypeInformation(
        name="request_type",
        encoding="raw",
        descriptor=b"")
RESPONSE_TYPE = DataTypeInformation(
        name="response_type",
        encoding="raw",
        descriptor=b"")
DEFAULT_TYPE = ecal_core.DataTypeInformation()

class TestServiceMethodInformation:
  def test_constructor_empty(self):
    method_info = ServiceMethodInformation()
    assert method_info.method_name == EMPTY_STRING
    assert method_info.request_type == DEFAULT_TYPE
    assert method_info.response_type == DEFAULT_TYPE
 
  def test_constructor_all(self):
    method_info = ServiceMethodInformation(
      method_name=METHOD_NAME,
      request_type=REQUEST_TYPE,
      response_type=RESPONSE_TYPE
    )
    assert method_info.method_name == METHOD_NAME
    assert method_info.request_type == REQUEST_TYPE
    assert method_info.response_type == RESPONSE_TYPE