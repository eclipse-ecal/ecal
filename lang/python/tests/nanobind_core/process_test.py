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

import pytest
import ecal.nanobind_core as ecal_core

def test_get_unit_name():
    unit_name = "Python Test"
    ecal_core.initialize(unit_name)
    assert ecal_core.process.get_unit_name() == unit_name   
    ecal_core.finalize()

def test_set_process_state():
    ecal_core.initialize()
    ecal_core.process.set_state(ecal_core.process.Severity.HEALTHY, ecal_core.process.SeverityLevel.LEVEL1, "I am doing fine")
    # we could check via monitoring that the state is set correctly. however, we're not doing that atm
    ecal_core.finalize()
