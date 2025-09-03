# Service Call Benchmark (eCAL + Google Benchmark)

This document explains a microbenchmark that measures request–response performance of an eCAL service call using Google Benchmark.

---

## Overview

- **Scenario:** A service **server** exposes a method named `ping`. A **client** repeatedly issues `CallWithResponse("ping", "request", ...)` and waits for the reply. The server responds immediately with a small constant payload (e.g., a short string).
- **Goal:** Measure the per‑call overhead of the eCAL RPC path within a single process (server and client run in separate threads under the same executable).
- **Timing:** Uses Google Benchmark’s **default timing** (CPU time). If you want wall‑clock latency, extend the code to use `UseRealTime()` or manual timing.
- **Startup sync:** Before timing, the benchmark waits for discovery/registration and verifies that at least one client instance is matched to the server.

> **Note:** This benchmark targets intra‑process/intra‑host behavior and does not isolate network or cross‑process effects.

---

## What the benchmark does

1. Initializes eCAL and constructs a **service server** named `Server` with a single method `ping`.
2. Registers a method callback that writes a short response and returns success.
3. Constructs a **service client** bound to `Server` and aware of the `ping` method.
4. Waits for discovery/registration, then verifies that a client instance is available; if not, waits again and aborts to avoid measuring an unconnected setup.
5. Enters the benchmark loop, repeatedly calling `CallWithResponse("ping", "request", timeout)` and waiting for the reply.
6. Finalizes eCAL at the end of the run.

---

## Configuration & knobs

Adjust the constants and parameters in the source to fit your environment:

- **Registration delay:** Initial discovery wait (default: ~2000 ms) before measurements start.
- **Service and method names:** Server name `Server`; method name `ping`.
- **Timeout:** The client uses a default call timeout argument; update as needed for your environment.
- **Payload content:** Response is a small constant string.
