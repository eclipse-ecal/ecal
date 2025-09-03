# Multi‑Topic Pub/Sub Background‑Load Benchmark (eCAL + Google Benchmark)

This document describes a microbenchmark that measures publish performance on a **foreground topic** while generating configurable **background traffic** on multiple other topics.

---

## Purpose

Quantify how **concurrent publishers** and **message sizes** affect the publish cost of a main topic under load. The benchmark varies:

- the **number of background topics** (`1, 2, 4, 8, 16, 32`), and
- the **per‑topic payload size** (`1 B`, `4 KiB`, `16 MiB`).

For each combination, the foreground publisher emits a payload of the same size while background threads continuously publish on their own topics. Timing uses **wall‑clock real time**.

---

## Benchmark at a glance

- **Foreground topic**: `benchmark_topic`
- **Background topics**: `background_topic_{i}` (one publisher + one subscriber per topic)
- **Argument matrix**: `topic_count ∈ {1..32} (×2)` × `payload_size ∈ {1, 4096, 16777216} bytes (×4096)`
- **Registration delay** before measurement: **2000 ms**
- **Warm‑up time** (Google Benchmark): **2 s**
- **Timing mode**: `UseRealTime()`

> All participants run in the **same process** (separate threads) and communicate via eCAL, targeting intra‑host behavior rather than cross‑process or cross‑host transport characteristics.

---

## How it works

1. **Initialization**: eCAL is initialized once at start.
2. **Background receivers**: A thread creates `topic_count` subscribers (`background_topic_0..N-1`) and keeps them alive.
3. **Background publishers**: `topic_count` threads each create a publisher on its own background topic, generate a payload of `payload_size`, wait for discovery, then **send in a tight loop** until the benchmark stops.
4. **Foreground pair**: A publisher on `benchmark_topic` is created and matched with a foreground subscriber in a helper thread.
5. **Synchronization**: The benchmark waits **2000 ms** to allow discovery/matching.
6. **Measurement**: In the Google Benchmark loop, the foreground publisher sends `payload_size` bytes every iteration; background threads keep publishing.
7. **Shutdown**: An atomic stop flag is raised, all threads join, and eCAL is finalized.

---

## Tuning & parameters

Adjust the following constants near the top of the source as needed:

- **`registration_delay_ms`** (default: 2000 ms)
- **`warmup_time_s`** (default: 2 s)
- **`background_topic_count_{min,max,multiplier}`** (default: 1 → 32, ×2)
- **`per_topic_range_{start,limit,multiplier}`** (default: 1 → 16 MiB, ×4096)