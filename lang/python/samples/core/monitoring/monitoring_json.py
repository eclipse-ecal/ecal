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

import base64
import sys
import time
import json

import ecal.core.core as ecal_core

def convert_bytes_to_str(d, handle_bytes='decode'):
  """
  Recursively converts all byte objects in a nested data structure to a string representation.
  
  Parameters:
    d: The input data structure, which can be a dictionary, list, tuple, set, or other types.
    handle_bytes (str or callable): Determines how bytes objects are handled:
      - 'decode': Encode bytes as a base64 string (default).
      - 'clear': Replace bytes with an empty string.
      - A callable: Apply the provided function to bytes objects.
  
  Returns:
    The input data structure with bytes objects converted according to the `handle_bytes` parameter.
  """  
  def handle_bytes_func(b):
    """
    Handles the conversion of a bytes object based on the `handle_bytes` parameter.
    
    Parameters:
      b: The bytes object to be handled.
      
    Returns:
      A string representation of the bytes object based on the `handle_bytes` parameter.
    
    Raises:
      ValueError: If the `handle_bytes` parameter is not recognized.
    """    
    if handle_bytes == 'decode':
      return base64.b64encode(b).decode('utf-8')
    elif handle_bytes == 'clear':
      return ''
    elif callable(handle_bytes):
      return handle_bytes(b)
    else:
      raise ValueError(f"Invalid handle_bytes value: {handle_bytes}")

  if isinstance(d, dict):
    return {k: convert_bytes_to_str(v, handle_bytes) for k, v in d.items()}
  elif isinstance(d, list):
    return [convert_bytes_to_str(i, handle_bytes) for i in d]
  elif isinstance(d, tuple):
    return tuple(convert_bytes_to_str(i, handle_bytes) for i in d)
  elif isinstance(d, set):
    return {convert_bytes_to_str(i, handle_bytes) for i in d}
  elif isinstance(d, bytes):
    return handle_bytes_func(d)
  else:
    return d

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # initialize eCAL API, including monitoring
  ecal_core.initialize("monitoring", ecal_core.INIT_ALL)
  time.sleep(2)
  
  # print eCAL entities
  while ecal_core.ok():
    # convert 'bytes' type elements of the the monitoring dictionary
    monitoring_d = convert_bytes_to_str(ecal_core.mon_monitoring(), handle_bytes='decode')
    # dump the dictionary into a json string
    json_s = json.dumps(monitoring_d, indent = 2) 
    print(json_s)
    time.sleep(5.0)
  
  # finalize eCAL monitoring API
  ecal_core.mon_finalize()
  
  # finalize eCAL API
  ecal_core.finalize()
 
if __name__ == "__main__":
  main()  
