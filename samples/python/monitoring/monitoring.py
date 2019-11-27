# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
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
import pprint

import ecal.core.core as ecal_core

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # initialize eCAL API
  ecal_core.initialize(sys.argv, "monitoring")
  
  # initialize eCAL monitoring API
  ecal_core.mon_initialize()
  time.sleep(2)
  
  # print eCAL entities
  while ecal_core.ok():
    pprint.pprint(ecal_core.mon_monitoring())
    time.sleep(5.0)
  
  # finalize eCAL monitoring API
  ecal_core.mon_finalize()
  
  # finalize eCAL API
  ecal_core.finalize()
  
if __name__ == "__main__":
  main()  
