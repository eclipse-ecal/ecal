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

import subprocess
import re
import json

# Run 7-Zip benchmark and capture output
try:
   output = subprocess.run(["7z", "b"], capture_output=True, text=True, check=True)
except FileNotFoundError as e:
   print(f"7-Zip executable not found: {e}")
   exit(1)
except subprocess.CalledProcessError as e:
   print(f"7-Zip exited with {e.returncode}. Stderr: {e.stderr}")
   exit(1)

# Extract total rating
score = int(re.findall(r'\d+', output.stdout)[-1])
print(f"7-Zip total score: {score}")

# Store in Bencher-style dictionary
result = { 
   "7z_score" : {
      "score" : {
         "value" : score
      }
   }
}

# Save in file
try:
   file_out= "7z_score.json"
   with open(file_out, "w") as f:
      json.dump(result, f, indent=4)
except IOError as e:
   print(f"ERROR: Failed to write output file: {e}")
   exit(1)