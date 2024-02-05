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

import ecal.measurement.hdf5 as ecalhdf5

import ecal.proto.helper as pb_helper

from enum import Enum

def combine_encoding_and_type(encoding, type):
  return encoding + ":" + type

class ProtobufChannelWriter:
    """Class to create Protobuf Channels

    This API is not considered stable and might break in future eCAL Versions
    """

    def __init__(self, channel_name: str, protobuf_type, meas: ecalhdf5.Meas):
        self._protobuf_type = protobuf_type
        self._binary_writer = BinaryChannelWriter(channel_name, meas=meas, encoding = self.encoding, type = self.type, descriptor = self.descriptor)

    def write_entry(self, data, rcv_timestamp: int, snd_timestamp: int):
        self._binary_writer.write_entry(data.SerializeToString(), rcv_timestamp, snd_timestamp)

    @property
    def name(self):
        return self._channel_name

    @property
    def descriptor(self):
        return pb_helper.get_descriptor_from_type(self._protobuf_type)

    @property
    def encoding(self):
        return "proto"

    @property
    def type(self):
        return self._protobuf_type.DESCRIPTOR.full_name

class BinaryChannelWriter:
    """Class to create Binary Channels
    
    This API is not considered stable and might break in future eCAL Versions
    """
    
    def __init__(self, channel_name: str, meas: ecalhdf5.Meas, encoding = "", type = "", descriptor = ""):
        self._meas = meas
        self._channel_name = channel_name
        self._encoding = encoding
        self._type = type
        self._descriptor = descriptor

        self._meas.set_channel_description(channel_name, descriptor)
        self._meas.set_channel_type(channel_name, combine_encoding_and_type(encoding, type))

    def write_entry(self, data, rcv_timestamp: int, snd_timestamp: int):
        self._meas.add_entry_to_file(data, rcv_timestamp, snd_timestamp, self._channel_name)

    @property
    def name(self):
        return self._channel_name

    @property
    def encoding(self):
        return self._encoding

    @property
    def type(self):
        return self._type

    @property
    def descriptor(self):
        return self._descriptor


class Encoding(Enum):
    BINARY = 0
    PROTOBUF = 1

class MeasurementWriter:
    """Write an eCAL Measurement
    
    This API is not considered stable and might break in future eCAL Versions
    """

    def __init__(self, output_dir: str, file_name: str, max_size_per_file: int):
        self._output_dir = output_dir
        self._file_name = file_name
        self._max_size_per_file = max_size_per_file
        self._channels = {}

        self._meas = ecalhdf5.Meas(output_dir, 1)
        self._meas.set_file_base_name(file_name)
        self._meas.set_max_size_per_file(max_size_per_file)

    def create_channel(self, channel_name: str, channel_encoding: Encoding, datatype):
        if channel_encoding == Encoding.PROTOBUF:
            channel = ProtobufChannelWriter(channel_name = channel_name, protobuf_type = datatype, meas = self._meas)
        self._channels[channel_name] = channel
        return channel

    def __del__(self):
        self._meas.close()
