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

import sys
import time

import ecal.nanobind_core as ecal_core

# eCAL receive callback
def callback(publisher_id, datatype_info, data):
  try:
    print(publisher_id)
    print(datatype_info)
    print(data)
  except Exception as e:
    print(e)
  
def event_callback(topic_id, data):
  try:
    print(topic_id)
    print(data)
  except Exception as e:
    print(e)
  
  
def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date_string()))
  
  # initialize eCAL API
  ecal_core.initialize()

  # create publisher
  config = ecal_core.get_subscriber_configuration()
  datatype_info = ecal_core.DataTypeInformation()
  datatype_info.encoding = "string"
  datatype_info.descriptor = b"abcd"
  
  # create subscriber and connect callback
  sub = ecal_core.Subscriber("Hello")
  sub.set_receive_callback(callback)
  
  # idle main thread
  while ecal_core.ok():
    time.sleep(0.1)
  
  # finalize eCAL API
  ecal_core.finalize()
  
if __name__ == "__main__":
  main()

