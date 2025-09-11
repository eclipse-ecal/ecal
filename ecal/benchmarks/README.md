# eCAL Benchmarks - Overview

This folder contains the benchmarks used in the performance tracking of eCAL (see GitHub workflows). The benchmarks use the Google Benchmark framework. Each sub-folder (except util) contains a source file and another Readme explaining the details of the benchmark(s) implemented in the source file.

---

# Build

- When building eCAL, the `-DECAL_THIRDPARTY_BUILD_BENCHMARK` and `-DECAL_BUILD_BENCHMARKS` options must be set to `ON`.
- Each sub-folder (except util) produces an independent executable with the benchmark(s) of the included source file when built.

---

# Structure

Overview over the sub-folders and which benchmarks are implemented in their source files:

- **pubsub**
   - Send
   - Send and Receive
   - Receive Latency
- **pubsub_config**
   - Send (with combinations of Zero Copy, Double Buffer and Handshake)
- **pubsub_multi**
   - Send with multiple publishers simultaneously
- **service**
   - Ping
- **setup**
   - Initialize
   - Initialize and Finalize
   - Publisher Creation
   - Subscriber Creation
   - Registration Delay
- **util**
   - Python script to run the 7-Zip compression/decompression benchmark and report back the total score
   - Python script to calculate frequency and datarate with the results of a PubSub benchmark