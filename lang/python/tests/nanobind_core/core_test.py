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

# get constants from git???
#ECAL_VERSION = "v6.0.0-nightly-52-g063aa0199-dirty"
#ECAL_DATE = "24.05.2024"
ECAL_VERSION_MAJOR = 6
ECAL_VERSION_MINOR = 0
ECAL_VERSION_PATCH = 0


def test_get_version():
    # Get eCAL version string
    ecal_core.get_version_string()

    # Get eCAL version date
    ecal_core.get_version_date_string()

    # Get eCAL version as separated integer values
    version = ecal_core.get_version()
    assert version.major == ECAL_VERSION_MAJOR
    assert version.minor == ECAL_VERSION_MINOR
    assert version.patch == ECAL_VERSION_PATCH


def test_initialize_finalize():
    # Is eCAL API initialized?
    assert ecal_core.is_initialized() is False

    # Initialize eCAL API
    assert ecal_core.initialize() is True

    # Is eCAL API initialized?
    assert ecal_core.is_initialized() is True

    # Initialize again
    assert ecal_core.initialize() is False

    # Finalize (should not fully finalize yet)
    assert ecal_core.finalize() is True
    assert ecal_core.is_initialized() is False

    # Finalize again (now full finalize)
    assert ecal_core.finalize() is False
    assert ecal_core.is_initialized() is False

def test_ecal_ok():
    # Check uninitialized
    assert ecal_core.ok() is False

    # Initialize
    assert ecal_core.initialize()
    assert ecal_core.ok() 

    # Finalize
    assert ecal_core.finalize()
    assert ecal_core.ok() is False
