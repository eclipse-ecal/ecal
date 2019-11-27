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

import sys
import random

import ecal.measurement.hdf5 as ecalhdf5

def main():
  """
   eCALHDF5 Write & Read Sample
   Step 1: create hdf5 file and add some data
   Step 2: read data from newly created file
  """

  ENTRY_COUNT = 30
  MAX_ENTRY_DATA_SIZE = 32767

  # Step 1: create hdf5 file and add some data

  # File properties
  output_dir = "ecalhdf5_rw_meas_folder"
  file_name  = "measurement"

  channels = []
  channels.append(ecalhdf5.Channel("Image",  b"Image description",  "Image type"))
  channels.append(ecalhdf5.Channel("Status", b"Status description", "Status type"))

  max_size_per_file = 500 # MB

  meas = ecalhdf5.Meas(output_dir, 1)
  meas.set_file_base_name(file_name)
  meas.set_max_size_per_file(max_size_per_file)

  for channel in channels:
    meas.set_channel_description(channel.name, channel.description)
    meas.set_channel_type(channel.name, channel.type)

  print("Creating {}/{}.hdf5 \n".format(output_dir, file_name))

  # Generate timestamps and random data and write them
  timestamp = 0
  for entry_no in range(ENTRY_COUNT):
    timestamp += 100000 # µs
    data_size = random.randint(1, MAX_ENTRY_DATA_SIZE)

    data = ""
    for i in range(data_size):
      data = data + chr(random.randint(0, 255))

    # randomly select to which channel to add the generated data  
    index = random.randint(0, MAX_ENTRY_DATA_SIZE) % len(channels)
    channel_to_write_to = channels[index].name

    print("   Entry {}\t{}\ttimestamp: {}\tsize[b]: {}".format(entry_no, channel_to_write_to, timestamp, data_size))

    # Write entry to file  
    meas.add_entry_to_file(str.encode(data), timestamp, timestamp, channel_to_write_to)

  print("\nTotal entries written: {}\n".format(ENTRY_COUNT))

  if meas.is_ok() == False:
    print("Write error!")
    sys.exit()

  meas.close()

  # Step 2: read data from newly created file

  print("******************************************************************************\n")

  meas = ecalhdf5.Meas(output_dir, 0)

  # Alternately single file can be used
  # meas = ecalhdf5.Meas(output_dir + "/" + file_name + ".hdf5", 0)
  # Both file and directory path are supported

  if meas.is_ok() == False:
    print("Read error!")
    sys.exit()

  print("Reading {}\n".format(output_dir))
  print("  File version:        {}".format(meas.get_file_version()))
  print("  Channels No:         {}\n".format(len(meas.get_channel_names())))
  print("  Channels:            \n")

  channel_names_set = meas.get_channel_names()
  for channel_name in channel_names_set:
    print("    Name:          {}".format(channel_name))
    print("    Type:          {}".format(meas.get_channel_type(channel_name)))
    print("    Description:   {}".format(meas.get_channel_description(channel_name)))
    print("    Min timestamp: {}".format(meas.get_min_timestamp(channel_name)))
    print("    Max timestamp: {}".format(meas.get_max_timestamp(channel_name)))

    entries_info_read = meas.get_entries_info(channel_name)
    print("    Entries count: {}\n".format(len(entries_info_read)))

    """
    # Alternately range entries info can be used, for example get the entries from the first timestamp until "the middle"    

    min_timestamp = meas.get_min_timestamp(channel_name)
    max_timestamp = meas.get_max_timestamp(channel_name)
    middle_timestamp = min_timestamp / 2 + max_timestamp / 2

    entries_info_read = meas.get_entries_info_range(channel_name, min_timestamp, middle_timestamp)

    print("    Entries count in timestamp interval [{}; {}]: {}\n\n".format(min_timestamp, middle_timestamp, len(entries_info_read)))
    print("    Reading entries info(timestamp ordered): \n")
    """

    for entry_read in entries_info_read:
      data_size = meas.get_entry_data_size(entry_read['id'])
      print("    Entry {}\tsnd_timestamp: {}\trcv_timestamp: {}\tsize[bytes]: {}".format(entry_read['id'], entry_read['snd_timestamp'], entry_read['rcv_timestamp'], data_size))
      entry_data = meas.get_entry_data(entry_read['id'])

    print("")

  meas.close()

if __name__ == "__main__":
  main()
