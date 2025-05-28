# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2024 Continental Corporation
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

from typing import TYPE_CHECKING, Type, NamedTuple
from enum import Enum

class BinaryChannelInfo(NamedTuple):
    topic_name: str
    type_encoding: str
    type_name: str
    type_descriptor: bytes

class BinaryEntry(NamedTuple):
    rcv_timestamp: int
    snd_timestamp: int
    msg: bytes

class BinaryChannel(object):

  def __init__(self, measurement, channel_name):
    self._measurement = measurement
    self._channel_name = channel_name
    self._entries = self._measurement._reader.get_entries_info(channel_name)

    channel_type = measurement._reader.get_channel_type(channel_name)
    try:
      self._type_encoding, self._type_name = channel_type.split(":", 1)
    except:
      self._type_name = channel_type
      self._type_encoding = ""
    self._type_descriptor = measurement._reader.get_channel_description(channel_name)

    #self.info = BinaryChannelInfo(topic = channel_name, encoding = "proto", schema_name = type, schema = descriptor)

  #@property
  #def info(self):
  #  return self.info
  
  @property 
  def topic_name(self) -> str:
    return self._channel_name

  @property
  def type_encoding(self) -> str:
    return self._type_encoding

  @property
  def type_name(self) -> str:
    return self._type_name

  @property
  def type_descriptor(self) -> bytes:
    return self._type_descriptor
  

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
    return BinaryChannel.Iterator(self)

  def __repr__(self):
    return "< Channel object: name: %s - number of entries: %i >" % (self._channel_name, len(self._entries))

  def __str__(self):
    return self.__repr__()

  def __len__(self):
    return len(self._entries)

  def __getitem__(self, entry_position):
    entry = self._entries[entry_position]

    rcv_timestamp = entry['rcv_timestamp']
    snd_timestamp = entry['snd_timestamp']
    id = entry['id']

    data = self._measurement._reader.get_entry_data(id)
    return BinaryEntry(rcv_timestamp = rcv_timestamp, snd_timestamp = snd_timestamp, msg = data)


class Channel(object):

  def __init__(self, measurement, channel_name):
    self._measurement = measurement
    self._channel_name = channel_name
    self._entries = self._measurement._reader.get_entries_info(channel_name)

    protocol, type = measurement._reader.get_channel_type(channel_name).split(":", 1)
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

  class ChannelAccessMode(Enum):
    OBJECT = 1
    BINARY = 2
    
  _channel_access_dict = {
    ChannelAccessMode.OBJECT : Channel, 
    ChannelAccessMode.BINARY : BinaryChannel
  }

  class Iterator(object):

    def __init__(self, measurement, channel_type):
      self._measurement = measurement
      self._iterator = iter(measurement.channel_names)
      self._channel_type =  channel_type
      
    def __next__(self):
      return self._channel_type(self._measurement, self._iterator.__next__())

    def next(self):
      return self.__next__()

  def __init__(self, path, channel_access_mode : ChannelAccessMode = ChannelAccessMode.OBJECT):
    self._reader = ecal_hdf5.Meas(path)
    self._path = path
    self._channel_type = self._channel_access_dict[channel_access_mode]
    

  @property
  def channel_names(self):
    return self._reader.get_channel_names()

  def __getitem__(self, channel_name):
    # TODO: check if channel_name exists
    return self._channel_type(self, channel_name)

  def __iter__(self):
    return Measurement.Iterator(self, self._channel_type)

  def __repr__(self):
    return "< Measurement object: path: %s - number of channels: %i >" % (self._path, len(self.channel_names))

  def __str__(self):
    return self.__repr__()

  def __del__(self):
    self._reader.close()

  def __len__(self):
    return len(self._measurement.channel_names)
