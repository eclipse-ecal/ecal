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
import os
import time

import ecal.nanobind_core as ecal_core

def main():
  ecal_core.initialize('Process Util Sample', ecal_core.init.ALL)

  # Print the eCAL data directory
  print("eCAL Data directory: {}".format(ecal_core.get_ecal_data_dir()))

  # Print the eCAL logging directory, where file logs will be placed
  print("eCAL Logging directory: {}".format(ecal_core.get_ecal_log_dir()))
  
  # Sets a shutdown signal to person_send, which will cause `eCAL::ok()` in that process to return false;
  ecal_core.shutdown_process_by_name("person_send")

  ecal_core.finalize()
  
if __name__ == "__main__":
  main()
