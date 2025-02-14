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


def main():
  ecal_core.initialize('logging', ecal_core.ALL)
  
  ecal_core.log(ecal_core.LogLevel.INFO, "Hello Hello")
  ecal_core.log(ecal_core.LogLevel.WARNING, "Help") 
  
  
  all_logging = ecal_core.get_logging()
  
  for log in all_logging.log_messages:
    print(log)  
  
  ecal_core.finalize()
  
  
  
if __name__ == "__main__":
  main()
