# ADR: 64-bit Entity ID Generation for eCAL

- Status: Accepted
- Date: 2026-03-24
- Deciders: eCAL maintainers
- Technical Story: Unique entity ID generation across processes, hosts, and containers with low configuration overhead

## Context

eCAL needs to generate `uint64_t` entity IDs that are unique across multiple processes and execution environments.

The practical deployment assumptions are:

- usually fewer than 10 distinct hosts / containers
- fewer than 1000 processes in the system
- fewer than 1000 IDs per process

The original implementation used:

- a per-process seed derived from PID, hostname, clocks, and random entropy
- a per-process atomic counter
- a 64-bit mixing function to generate the final ID

That design provides very low collision probability, but its uniqueness model is not obvious from the bit pattern and depends on multiple heuristics for process seed generation.

We want a simpler and more explainable default that:
- works without manual configuration
- performs well
- keeps the implementation compact
- provides very low collision probability across hosts, containers, and process restarts

## Decision

eCAL will generate entity IDs using a structured 64-bit layout:

```text
[ process_namespace:24 | pid:24 | counter:16 ]