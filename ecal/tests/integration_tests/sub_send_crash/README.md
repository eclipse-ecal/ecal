# sub_send_crash – Crash During Reception Test

## Overview

This test case evaluates the robustness of the eCAL system when a subscriber crashes while receiving a large message. It verifies whether other subscribers and the publisher remain unaffected.

It also includes a variant that uses Shared Memory (SHM) transport with Zero-Copy enabled, a performance-critical feature for high-throughput systems.

---

## Test Objective

- Validate that the system remains stable when one subscriber crashes during message reception.
- Assess how the system behaves under large message loads (e.g., 50MB–1GB).
- Verify the effectiveness of Zero-Copy Shared Memory under fault conditions.

---

## Test Scenario

1. A `large_publisher` sends large messages (default: 50MB).
2. A `crash_subscriber` receives a few messages and then crashes during callback.
3. A `test_subscriber` runs normally and must complete successfully.
4. In the `zero_copy` variant:
   - The publisher uses `zero_copy_mode = true` in SHM.
   - All binaries are executed inside one container for local SHM tests.
5. In `network_*` modes, each binary is run in a separate container.
6. In `local_*` modes, each binary is run in one container.

---

## Success Criteria

- The `test_subscriber` receives all messages and exits with status code `0`.
- The `crash_subscriber` crashes during reception (intentionally).
- The `large_publisher` is not blocked by the crash and completes transmission.
- In `zero_copy` mode, eCAL's memory sharing mechanism should function correctly even when a subscriber crashes while holding the shared memory segment.

---

## Transport Layer Notes

| Transport Mode     | Status        | Notes                                                                 |
|--------------------|---------------|-----------------------------------------------------------------------|
| `local_shm`        | ✅ Tested      | Fails if `zero_copy` is enabled                                      |
| `local_udp`        | ❌ Skipped     | UDP cannot transmit large messages , not suited for this test        |
| `local_tcp`        | ✅ Tested      | Works                                                                |
| `network_udp`      | ✅ Tested      | Works                                                                |
| `network_tcp`      | ✅ Tested      | Works                                                                |

---

## Running the Test

Run all test cases:

```bash
robot robottests/sub_send_crash.robot
