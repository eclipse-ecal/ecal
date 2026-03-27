# ADR: 64-bit Entity ID Generation for eCAL

- Status: Accepted
- Date: 2026-03-24
- Deciders: eCAL maintainers
- Technical Story: Unique entity ID generation

## Context

eCAL needs to generate `uint64_t` entity IDs that are unique across multiple processes and execution environments.
The original implementation used only the system timestamp, so there was a collision risk, if many processes / entities are started simultaneously.

The practical deployment assumptions are:
- usually fewer than 10 distinct hosts / containers
- usually fewer than 1000 processes in the system
- usually fewer than 10000 IDs per process

However, processes may not coordinate among themselves, so an algorithm with a low enough collision propability needs to be selected

## Decision

eCAL will generate entity IDs using a structured 64-bit layout:

```text
[ pid:24 + process_namespace:24 + counter:16 ]
```

- 24 bit PID: PID may be up to 22 bit on Unix systems (per configuration), on Windows systems they are usually numbers < 2^16, although the type returned by the OS is uint32_t
- 24 bit Process namespace: PID is only guaranteed to be unique within one host. This is why a random 24 bit value for the process namespace is introduced
- 16 bit Counter: Unique per process and incrementing. The type of the counter however is larger, and will "leak" into the process namespace if it's higher than 2^16.

