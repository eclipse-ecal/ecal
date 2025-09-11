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