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
  ecal_core.initialize('Process Python Sample', ecal_core.init.ALL)

  # We can query the eCAL Unit name which was set via `initialize`
  unit_name = ecal_core.process.get_unit_name()
  print("The name of this unit is '{}'".format(unit_name))
  
  # We can set a process state that will be shown in eCAL Monitor and eCALSys.
  ecal_core.process.set_state(ecal_core.process.Severity.HEALTHY, ecal_core.process.SeverityLevel.LEVEL1, "I am doing fine")
  
  time.sleep(10)

  ecal_core.finalize()
  
if __name__ == "__main__":
  main()
