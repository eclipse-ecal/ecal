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
import time

import ecal.nanobind_core as ecal_core

def main():
  ecal_core.initialize('Monitoring Python Sample', ecal_core.init.ALL)
  
  try:
    while ecal_core.ok():
        #TODO: is the naming good?
        # or ecal_core.monitoring.
        monitoring = ecal_core.monitoring.get_monitoring()
        
        print("There are {} publishers and {} subscribers in the system".format(len(monitoring.publishers), len(monitoring.subscribers)))

        serialized_monitoring = ecal_core.get_serialized_monitoring()
        print("Length of serialized monitoring: {}".format(len(serialized_monitoring)))
        # You could now use protobuf to deserialize the monitoring string
        # We recommend to use the `get_monitoring` instead.

        time.sleep(1)
            
  except KeyboardInterrupt:
    pass
  finally:  
    ecal_core.finalize()
  
if __name__ == "__main__":
  main()
