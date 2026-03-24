# ADR-0001: Entity ID generation for eCAL core communication

- **Status:** Accepted
- **Date:** 2026-03-24

## Context

eCAL entities such as publishers, subscribers, service clients, and service servers require an entity id.
Historically, several entities used:

```cpp
std::chrono::steady_clock::now().time_since_epoch().count()
```

This can collide when entities are created in parallel or when multiple processes start at nearly the same time.

## Decision

Introduce a private utility `eCAL::Util::GenerateUniqueEntityId()` in
`ecal/core/src/util/entity_id_generator.h` and use it at entity-id assignment sites.

Design:

1. Build a deterministic per-process seed from:
   - process id
   - 64-bit FNV-1a hash of hostname
2. Maintain a process-local `std::atomic<std::uint64_t>` counter.
3. Generate the ID as a splitmix64 permutation over `process_seed + counter`.

## Consequences

### Positive

- Removes direct dependency on clock ticks for id generation.
- Guarantees uniqueness per process until counter wrap-around.
- Distinguishes concurrently running processes by process identity `(host, pid)`.
- Fast, lock-free generation path.

### Trade-offs

- System-wide uniqueness is still bounded by 64-bit space and host-hash collision probability.
- The approach assumes hostname and pid together represent process identity during overlap.
- Absolute global uniqueness across all time and all environments would require a coordinated allocator or larger ID space.

## Alternatives considered

### Clock-based IDs

Rejected due to collision risk under high parallelism.

### Pure random 64-bit IDs

Rejected as the sole strategy because collision probability grows with the number of generated IDs (birthday paradox).

### UUID / centrally coordinated ID service

Not selected due to additional complexity and integration cost for core internals.

