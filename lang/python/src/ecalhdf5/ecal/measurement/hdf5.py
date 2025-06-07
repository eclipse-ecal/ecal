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

"""
  @package eCALHDF5
  Python wrapper for eCALHDF5

  You can use the eCALHDF5 API in a function style or OO manner.
"""

import importlib
import logging
import platform
import sys
import struct

import ecal._ecal_hdf5_py as _ecalhdf5

class Meas():

  def __init__(self, path = "", access = 0):
    self.meas = _ecalhdf5.Meas()
    self.meas.open(path, access)

  def open(self, path, access):
    return (self.meas.open(path, access))

  def close(self):
    return(self.meas.close())

  def is_ok(self):
    return (self.meas.is_ok())

  def get_file_version(self):
    return(self.meas.get_file_version())

  def get_max_size_per_file(self):
    return(self.meas.get_max_size_per_file())

  def set_max_size_per_file(self, size):
    return(self.meas.set_max_size_per_file(size))

  def get_channel_names(self):
    return(self.meas.get_channel_names())

  def get_channel_description(self, channel_name):
    return(self.meas.get_channel_description(channel_name))

  def set_channel_description(self, channel_name, description):
    return(self.meas.set_channel_description(channel_name, description))

  def get_channel_type(self, channel_name):
    return(self.meas.get_channel_type(channel_name))

  def set_channel_type(self, channel_name, type):
    return(self.meas.set_channel_type(channel_name, type))

  def get_min_timestamp(self, channel_name):
    return(self.meas.get_min_timestamp(channel_name))

  def get_max_timestamp(self, channel_name):
    return(self.meas.get_max_timestamp(channel_name))

  def get_entries_info(self, channel_name):
    return(self.meas.get_entries_info(channel_name))

  def get_entries_info_range(self, channel_name, begin, end):
    return(self.meas.get_entries_info_range(channel_name, begin, end))

  def get_entry_data_size(self, entry_id):
    return(self.meas.get_entry_data_size(entry_id))

  def get_entry_data(self, entry_id):
    return(self.meas.get_entry_data(entry_id))

  def set_file_base_name(self, base_name):
    return(self.meas.set_file_base_name(base_name))

  def add_entry_to_file(self, data, snd_timestamp, rcv_timestamp, channel_name, counter = 0):
    return(self.meas.add_entry_to_file(data, snd_timestamp, rcv_timestamp, channel_name, counter))


class Channel:

  def __init__(self, name, description, type):
    self.name = name
    self.description = description
    self.type = type
