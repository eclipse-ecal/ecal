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
ap.add_argument('-f', '--file', required=True, help="Path to the the log file. Can be relative to this python file.")
args = ap.parse_args()
file_in_path = args.file

# Check if file exists
if not os.path.isfile(file_in_path):
   print(f"ERROR: File not found at path: {file_in_path}")
   exit(1)

# Decode JSON file
try:
   with open(file_in_path, 'r') as f:
      data = json.load(f)
except json.JSONDecodeError as e:
   print(f"ERROR: Failed to parse JSON file: {e}")
   exit(1)

# Check if benchmark data exists
if "benchmarks" not in data or not isinstance(data["benchmarks"], list):
   print("ERROR: 'benchmarks' key missing or not a list in the JSON file.")
   exit(1)

# Extract benchmark data
benchmarks = data["benchmarks"]
# Create list for the results
full_results = {}

for itm in benchmarks:
   try:
      # Get payload size (in byte) from benchmark name
      payload_size = int(re.search(r'/(\d+)/[a-z]', itm["name"]).group(1))
      # Get background thread count from benchmark name (if applicable)
      thread_count = 1
      multi = re.search(r'[a-z]/(\d+)/[0-9]', itm["name"])
      if multi:
         # Adding 1 to account for the main thread
         thread_count += int(multi.group(1))
      # Get time taken. Expecting time in nanoseconds
      real_time_ns = float(itm["real_time"])
      # Calculating send frequency in hertz (corresponds to throughput in ops/s)
      frequency = 1 / (real_time_ns * 10**-9) * thread_count
      # Calculating speed in bytes per second
      speed = frequency * payload_size * thread_count
      # Output to console
      print(f"Payload Size: {payload_size} Bytes  ||  Real Time: {real_time_ns} ns  ||  Frequency: {frequency} ops/s  ||  Datarate: {speed} Bytes/s  ||  Thread count: {thread_count}")
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
   except (KeyError, ValueError, AttributeError) as e:
      print(f"WARNING: Skipping invalid benchmark entry: {itm}. Reason: {e}")

# Write full calculation results into a new json file
filename_no_ext = re.sub(r'\.[^.]+$', "", os.path.basename(file_in_path))
file_out = f"{filename_no_ext}_throughput-calculation.json"
try:
   with open(file_out, "w") as f:
      json.dump(full_results, f, indent=4)
except IOError as e:
   print(f"ERROR: Failed to write output file: {e}")
   exit(1)

print(f"Results written to {os.path.abspath(file_out)}")