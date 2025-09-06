# eCAL Setup & Lifecycle Benchmarks

This document describes microbenchmarks that measure initialization, teardown, and entity-creation costs for an eCAL-based publish/subscribe setup.

---

## Overview

The suite registers five cases. All use the topic name `benchmark_topic` when creating publishers/subscribers and build into a standalone Google Benchmark binary via `BENCHMARK_MAIN()`.

1. **Initialize**  
   Measures the cost of repeatedly calling the middleware initialization routine.

2. **Initialize_and_Finalize**  
   Measures combined cost of initializing and immediately finalizing the middleware per iteration.

3. **Publisher_Creation**  
   Measures the cost of constructing a publisher object (initialization happens once before the loop; finalization after the loop).

4. **Subscriber_Creation**  
   Measures the cost of constructing a subscriber object (initialization happens once before the loop; finalization after the loop).

5. **Registration_Delay**  
   Measures discovery/registration time until a newly created publisher detects at least one subscriber. Publisher and subscriber are created outside timing using `PauseTiming/ResumeTiming`; the measured section spins until `GetSubscriberCount() > 0`. This case enforces a minimum benchmark duration of **5 seconds** to ensure multiple iterations.