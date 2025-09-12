# eCAL Benchmark Utility Scripts

This document describes the `7z_score.py` and the `throughput_calculator.py` python scripts.

---

# 7z-Score Python Script

# Purpose

This is script will run the 7-Zip compression/decompression benchmark and report back the total score. It is made to simply and quickly assess the performance of a given system. In our case, we use it to automatically assess the performance of GitHub runners in the scope of cross-referencing benchmark results from these runners.

---

# How it works

- **Running the 7-Zip Benchmark**: The script calls `7z b` to run the 7-Zip compression/decompression benchmark. To work, 7-Zip must be installed and `7z` known as a path variable.
- **Extracting the total score**: From the benchmark results, the script takes only the **total score** value (`Tot`), which is the average of the compression and decompression ratings over different dictionary sizes (**higher is better**). Refer to the [7-Zip Documentation](https://documentation.help/7-Zip/bench.htm) for further information.
- **Output**: Console output of the total score as well as storing it in a JSON file (see below).

---

# Output

The script creates a new JSON file with the name of `7z_score.json`. The file contains the total score value in a structure that makes it readable by Bencher (bencher.dev), our tool to store and display benchmark results:
```
"7z_score" : {
   "score" : {
      "value" : score
   }
}
```

---

# Throughput Calculator Python Script

# Purpose

The eCAL benchmarks measure latency, i.e. time taken to execute certain actions. The publisher-subscriber benchmarks also use different message sizes in the benchmarks. With these two data points (time and message size) this script calculates two other data points: **frequency** and **datarate**.

---

# How to use

> **Note**: The script is tailored to work in the environment of the automated benchmarking routine in the eCAL GitHub repository, so it expects a specific input and produces a specific output, see below.

**Executing the script**: Call the script with the `-f` or `--file` parameter and pass the path to the input file. The path can be relative to the python file.

> **Example**: `throughput_calculator.py -f /path/to/file`

---

# Input prerequisites

- The script is made to take JSON log files as produced by benchmark based on the **Google Benchmark** framework. 
- Input JSON files need to contain a set with the name of `benchmarks`. 
- Each member needs to have the properties `name` and `real_time`.
- The unit of `real_time` needs to be nanoseconds.
- The name of the analysed benchmark member needs to be of the format `$common_benchmark_name$/$benchmark_message_size$/$additional_benchmark_options$` or `$common_benchmark_name$/$number_of_background_threads$/$benchmark_message_size$/$additional_benchmark_options$`.

---

# How it works

- **Collecting data**: The script extracts the name, the time taken, the message size and (if applicable) the background thread count for each benchmark in the log file.
- **Calculating the frequency**: `frequency = 1 / (real_time_ns * 10**-9) * thread_count`, unit: Hertz
- **Calculating the datarate**: `speed = frequency * payload_size * thread_count`, unit: Bytes per second
- **Output**: Console output of the calculated values as well as a new JSON log file (see below).

> **Note**: In the scenarios of the `pubsub_multi` benchmark, where multiple threads are active in publishing messages, the **combined values** for frequency and datarate are calculated, i.e. the frequency/datarate of all active threads added together.

---

# Output

The script creates a new JSON file with the name of `$name_of_the_input_file$_throughput-calculation.json`. The file contains the calculated values in a structure that makes them readable by Bencher (bencher.dev), our tool to store and display benchmark results:
```
"$benchmark_name$" : {
   "throughput" : {
      "value" : frequency
   },
   "speed" : {
      "value" : speed
   }
}
```