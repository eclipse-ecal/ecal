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

  # For pure logging, it's not necessary to modify the configuration.
  # However, in order to also receive logging information it's necessary to be turned on
  config = ecal_core.init.get_configuration()
  config.logging.receiver.enable = True
  config.logging.provider.udp.log_levels = [ecal_core.LogLevel.INFO, ecal_core.LogLevel.WARNING, ecal_core.LogLevel.ERROR, ecal_core.LogLevel.FATAL]
  ecal_core.initialize(config, 'Logging Python Sample', ecal_core.init.ALL)
  
  ecal_core.log(ecal_core.LogLevel.INFO, "Hello Hello")
  ecal_core.log(ecal_core.LogLevel.WARNING, "Help") 
  
  all_logging = ecal_core.get_logging()
  
  for log in all_logging:
    print(log)
    print("\n")
  
  ecal_core.finalize()
  
  
  
if __name__ == "__main__":
  main()
