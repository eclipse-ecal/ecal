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

import argparse
import os
import json
import re


# Read log file path from argument
ap = argparse.ArgumentParser()
ap.add_argument('-f','--file',help='Path to the the log file. Can be relative to this python file.')
args = ap.parse_args()
file_path = args.file

# Do the calculations
with open(file_path, 'r') as f:
   data = json.load(f)

# Extract context
context = data["context"]
# Extract benchmark data
benchmarks = data["benchmarks"]
# Create list for the results
full_results = {}

for itm in benchmarks:
   # Get payload size (in byte) from benchmark name
   payload_size = int(re.search(r'/(\d+)/', itm["name"]).group(1))
   # Get time taken. Expecting time in nanoseconds
   real_time_ns = float(itm["real_time"])
   # Calculating send frequency in hertz (corresponds to throughput in ops/s)
   frequency = 1 / (real_time_ns * 10**-9)
   # Calculating speed in byte per second
   speed = frequency * payload_size
   # Create new dictionary for this datapoint
   datapoint = {
      "throughput" : {
         "value" : frequency
      },
      "speed" : {
         "value" : speed
      }
   }
   # Add dictionary to full results dictionary
   full_results.update({itm["name"] : datapoint})

# Write full calculation results into a new json file
filename_no_ext = re.sub(r'\.[^.]+$', "", os.path.basename(file_path))
with open(f"{filename_no_ext}_throughput-calculation.json", "w") as file_out:
   json.dump(full_results, file_out, indent=4)

print("SUCCESS")