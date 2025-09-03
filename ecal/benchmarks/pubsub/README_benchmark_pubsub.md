# eCAL Pub/Sub Benchmark

This README documents the `benchmark_pubsub.cpp` microbenchmarks that measure eCAL publish/subscribe performance within a single process.

---

## Overview

The file defines **three Google Benchmark** tests that use **eCAL** to exchange byte payloads on the topic `benchmark_topic`:

1. **Send/Bandwidth** — Measures the cost of *publishing* messages only (a receiver thread is present to keep the publisher matched, but its latency is not measured). Uses wall-clock time (`UseRealTime`).
2. **Send + Receive/Round-trip (1→1)** — Measures the cost of publishing a message and blocking until a *receive callback* fires. Uses wall-clock time (`UseRealTime`).
3. **Receive Latency (manual timing)** — Publishes a message and measures *delivery latency* from immediately before blocking to the callback time stamp. Uses manual timing (`UseManualTime` + `SetIterationTime`).  

All three benchmarks sweep the payload size from **1 B** to **16 MiB** with a multiplier of **×64** (`1, 64, 4096, 262144, 16777216` bytes). They use a 2 s registration delay to let publisher/subscriber match before timing starts.

---

## What the code does

- Initializes eCAL with `eCAL::Initialize("Benchmark")`, creates a `CPublisher` on `benchmark_topic`, and spawns a receiver thread with a `CSubscriber` on the same topic.
- **Payload generation:** For each selected size, a `std::vector<char>` is filled with random bytes via `std::mt19937` and `uniform_int_distribution<0,255>`. 
- **Send benchmark:** In the benchmark loop, `publisher.Send(content_ptr, payload_size)` is invoked; timing is handled by Google Benchmark (`UseRealTime`).  citeturn1search1
- **Send+Receive benchmark:** Before each iteration, a flag is reset; after `Send`, the code blocks on a condition variable that the subscriber callback sets to signal message arrival.
- **Receive Latency benchmark:** Timestamps are taken around the blocking wait (`time_start` before waiting; callback stores `time_end`). The iteration time is set to `time_end - time_start` via `state.SetIterationTime(...)` and `UseManualTime`.
- **Ranges & timing modes:**
  - Range multiplier: `1 << 6` (64)
  - Range: `1 .. (1 << 24)` bytes (1 B … 16 MiB)
  - Registration delay: `2000 ms`
  - Sleep in receiver loop: `10 ms`

> **Note:** Publisher and subscriber live in the *same process* (different threads). This targets intra-host (and potentially intra-process) behavior rather than multi-process or networked scenarios.

---

## Configuration & knobs

- **Topic name:** `benchmark_topic`  
- **Registration delay:** 2000 ms before measuring to allow discovery/matching  
- **Receiver loop sleep:** 10 ms  
- **Payload sizes:** 1 B → 16 MiB (×64 steps)  