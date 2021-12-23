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
  Python API for simplified access to the protobuf content of an eCAL measurement

  You can use the eCALHDF5 measurement API to extract (deserialize) the protobuf content from an eCAL HDF5 file.
"""

import ecal.measurement.hdf5 as ecal_hdf5
import ecal.proto.helper as pb_helper

class Channel(object):

  def __init__(self, measurement, channel_name):
    if channel_name not in measurement.channel_names:
      raise KeyError("Channel "+ channel_name + " does not exist in measurement.")
    
    self._measurement = measurement
    self._channel_name = channel_name
    self._entries = self._measurement._reader.get_entries_info(channel_name)

    protocol, type = measurement._reader.get_channel_type(channel_name).split(":")
    if protocol == "proto":
      self._proto_msg_class = pb_helper.get_type_from_descriptor(type, measurement._reader.get_channel_description(channel_name))
    else:
       raise NotImplementedError
    
  class Iterator(object):

    def __init__(self, channel):
      self._channel = channel
      self._position = 0

    def __next__(self):
      if self._position >= len(self._channel):
        raise StopIteration
      else:
        self._position += 1
      return self._channel[self._position - 1]

    def next(self):
      return self.__next__()

  def __iter__(self):
    return Channel.Iterator(self)

  def __repr__(self):
    return "< Channel object: name: %s - number of entries: %i >" % (self._channel_name, len(self._entries))

  def __str__(self):
    return self.__repr__()

  def __len__(self):
    return len(self._entries)

  @property 
  def proto_msg_class(self):
    return self._proto_msg_class

  def __getitem__(self, entry_position):
    entry = self._entries[entry_position]

    timestamp = entry['rcv_timestamp']
    id = entry['id']

    data = self._measurement._reader.get_entry_data(id)
    proto_msg = self._proto_msg_class()
    proto_msg.ParseFromString(data)

    return (timestamp, proto_msg)

class Measurement(object):

  class Iterator(object):

    def __init__(self, measurement):
      self._measurement = measurement
      self._iterator = iter(measurement.channel_names)

    def __next__(self):
      return Channel(self._measurement, self._iterator.next())

    def next(self):
      return self.__next__()

  def __init__(self, path):
    self._reader = ecal_hdf5.Meas(path)
    self._path = path

  @property
  def channel_names(self):
    return self._reader.get_channel_names()

  def __getitem__(self, channel_name):
    return Channel(self, channel_name)

  def __iter__(self):
    return Measurement.Iterator(self)

  def __repr__(self):
    return "< Measurement object: path: %s - number of channels: %i >" % (self._path, len(self.channel_names))

  def __str__(self):
    return self.__repr__()

  def __del__(self):
    self._reader.close()
