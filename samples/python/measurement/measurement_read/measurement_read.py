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

import os
import sys
import time

import ecal
from ecal.measurement.measurement import Measurement, Channel

def main():
  # Create a measurement (pass either a .hdf5 file or a measurement folder)
  measurement = Measurement("../../data/person")
  
  # Retrieve the channels in the measurement by calling measurement.channel_names
  print("The following channels are contained in the measurement: {}".format(measurement.channel_names));
  
  # You can iterate over all channels in a measurement
  for channel in measurement.channel_names:
    print("Channel {}:".format(channel))
    channel_content = measurement[channel]
  
    # Iterate through all messages in a channel, iterator will return a tuple (timestamp, message)
    for (timestamp, message) in channel_content:
      print("Channel {} at timestamp {}:".format(channel, timestamp))
      print(message)

if __name__ == "__main__":
  main()  
