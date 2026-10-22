# Publisher transport selection

Status: recovered behavior and proposed contracts. Fixed selection and separate
eligibility/preference are user-confirmed directions (2026-10-21); detailed
contracts still require review.

## Scope and baseline

Baseline: `29ad9f088a7e262982b0a97d76bb9eb9a5a5cb18`.
The inspected implementation files have no tracked worktree modifications.
Evidence below is source-derived, not runtime-observed.

Scope: choose a transport for one subscriber registration from publisher and
subscriber capabilities and an ordered publisher preference list.
Transport startup, delivery guarantees, discovery filtering, SHM domain
compatibility, and connection lifecycle are outside the pure decision slice.

## Evidence

| ID | Source | Relevant symbol or location |
| --- | --- | --- |
| E1 | [publisher implementation](../../ecal/core/src/pubsub/ecal_publisher_impl.cpp#L915) | `CPublisherImpl::DetermineTransportLayer` |
| E2 | [registration handling](../../ecal/core/src/pubsub/ecal_publisher_impl.cpp#L452) | `CPublisherImpl::ApplySubscriberRegistration` |
| E3 | [configuration defaults](../../ecal/core/include/ecal/config/publisher.h#L141) | `Publisher::Configuration` |
| E4 | [attribute builder](../../ecal/core/src/pubsub/config/builder/writer_attribute_builder.cpp#L26) | `BuildWriterAttributes` |
| E5 | [configuration tests](../../ecal/tests/cpp/config_test/src/yaml_processing_test.cpp#L80) | Priority round-trip examples; not selector conformance |

## Recovered behavior

| ID | Source-derived claim | Evidence |
| --- | --- | --- |
| OBS-TS-001 | Host-name string equality chooses local versus remote priorities. | E2 |
| OBS-TS-002 | Candidate lists include transports enabled at runtime and compiled into the publisher. Subscriber candidates use advertised read-enabled flags under the same build guards. | E2 |
| OBS-TS-003 | The first priority entry present in both candidate lists wins; no match yields `none`. List omission excludes a transport even if both peers enable it. | E1 |
| OBS-TS-004 | Default local order is SHM, UDP, TCP; default remote order is UDP, TCP. The helper itself does not prohibit remote SHM. | E1, E3 |
| OBS-TS-005 | Priorities are copied from publisher configuration into writer attributes. The helper does not consult network mode or SHM domain. | E1, E4 |
| OBS-TS-006 | An unmapped enum value encountered during scanning throws through `map::at`; an earlier match can prevent reaching it. All currently named enum values are mapped; this concerns out-of-range values, not a known runtime failure. | E1 |

For valid transport preferences, the decision can be expressed as:

```text
preferences := same_host ? local_preferences : remote_preferences
result := first member of preferences also in publisher_enabled and subscriber_enabled
result := none if no such member exists
```

Duplicate preferences do not change this result. Empty preferences yield `none`.
The registration caller never adds `none` to its candidate lists.

## Proposed target contracts

All requirements below are **proposed**, not accepted compatibility guarantees.
The target uses one defined preference order and pairwise eligibility rather
than choosing a local/remote preference list. Transport domains are per-transport
compatibility contexts, not necessarily host names or a global domain shared by
all transports. A matching SHM domain must denote genuinely accessible shared
resources, not merely equal arbitrary labels.

| ID | Proposed contract | Verification |
| --- | --- | --- |
| TS-001 | Given one validated preference order and the pairwise eligible transport set, the selector MUST return the first eligible preference, or `none` when there is no match. | Exhaustive finite-set C++ tests written; execution blocked by build configuration |
| TS-002 | The selector function MUST be a pure deterministic decision: no I/O, timers, waits, writer startup, or connection-state mutation. The surrounding registration operation is not required to be pure. | Boundary review and unit tests |
| TS-003 | A selected transport MUST belong to the pairwise eligible set and the preference order. There MUST be no implicit fallback outside the order. | Property tests |
| TS-004 | Eligibility MUST be computed from compatible advertised sender/receiver capabilities and transport-specific domain/access constraints. A peer MUST NOT advertise a capability it cannot provide. Administrative restrictions, if retained, MUST constrain advertised capabilities rather than require duplicated peer configuration. | Eligibility and advertisement tests, once domain policy is agreed |
| TS-005 | Invalid transport values MUST be rejected at the preference-input boundary with a diagnostic, before selection; input validity MUST NOT depend on which preference matches first. | Invalid input cases, once error API is agreed |
| TS-006 | Once committed to a connection, its selected transport MUST remain unchanged. A capability update or repeated discovery announcement MUST NOT switch that connection's transport. | Lifecycle event tests |

Caller-owned buffers, concurrency of publisher methods, delivery success,
latency bounds, and startup failures are not specified by these requirements.
The proposed selector has no mutable shared state. Exact public errors and
compatibility constraints remain decisions, not inferred guarantees.

## Simplification and decisions

Prefer one eligibility calculation followed by one ordered-set decision.
Do not introduce a transport-selection framework or a new negotiation protocol
for this rule. A small pure function is the candidate implementation boundary;
its concrete API is not yet decided.

| ID | Finding or question | Proposed direction / status |
| --- | --- | --- |
| DEC-TS-001 | Host-name equality is not a complete SHM reachability policy. | User direction: pairwise, transport-specific eligibility plus one preference order. Domain representation and preference authority remain unresolved; eligibility is not proof of readiness. |
| DEC-TS-002 | Invalid enum handling can depend on preference order. | Reject invalid preference inputs before use (TS-005), intentionally changing incidental exception behavior. Error API and configurable versus fixed order remain unresolved. |
| DEC-TS-003 | Selection and connection lifecycle currently interact. | Specify the lifecycle separately before changing it; assess whether selected layer, connection state, and counters duplicate information. |
| DEC-TS-004 | Should a capability or preference change reselect a live connection? | User decision: no runtime switching. Failure/reconnection policy must not silently bypass this by creating a replacement on another transport. See the lifecycle draft. |

E2 inserts a new connection as `pending`; a subsequent registration moves a
pending entry to `established`. For an existing entry it updates metadata and
layer states but does not replace the stored selected layer or adjust its layer
counter, even if the newly computed choice differs. This is a concrete lifecycle
review target, **not proof of a bug or the cause of any reported flaky test**.
Whether registrations may legitimately change capabilities is not established.

## Conformance status and next checks

The companion JSON examples exercise the recovered two-list selector and
selected edge cases, not the revised single-order target contracts. They
describe selector inputs, not network integration scenarios.
The remote-SHM example deliberately records the current helper's behavior;
future eligibility policy may exclude that input before calling the selector.

### Isolated C++ prototype

[transport_selection.h](../../ecal/core/src/pubsub/transport_selection.h)
implements two independent operations without integration into the publisher:

```text
eligible[transport] := sender_supports[transport]
                      AND receiver_supports[transport]
                      AND pair_compatible[transport]
selected := first eligible member of one supplied preference order, else none
```

The caller supplies directional sender/receiver capabilities and pairwise
compatibility facts. Computing those facts from domain descriptors, protocol
versions, and access policy is deliberately not implemented. Compatibility is
not inferred from host names, and eligibility is not a readiness guarantee.
There are no default preferences; authority and advertised representations
remain integration decisions. The existing two-list publisher is unchanged.

The prototype rejects `none` and out-of-range enum values anywhere in its
preference input before making a selection, using `std::invalid_argument`.
Duplicates are harmless and empty preferences return `none`. This is an
internal prototype API, not an agreed public exception contract. Validation
currently occurs at the selector entry; a future configuration/advertisement
boundary can validate earlier. The enum is reused from the existing API.

[C++ tests](../../ecal/tests/cpp/transport_selection_test/src/transport_selection_test.cpp)
in target `test_transport_selection` cover 512 sender/receiver/compatibility-set
combinations, all six preference permutations and their four prefixes (12,288
selection iterations, including repeated equivalent lists). The bitmask-based
oracle checks eligibility separately from selection. Additional tests cover
duplicates, omitted fallback, invalid values after an eligible entry, `none`,
and invalid input with an empty eligible set. No network or clock is required.

Verification status: source review and editor diagnostics passed. The CMake
build attempt stopped during configuration because no usable Qt installation
was found, before this target compiled. C++ tests are written but **not run**.
Only the original JSON decision examples and documentation consistency checks
have executed successfully. TS-004's real eligibility facts and advertisement
rules, and TS-006's lifecycle invariants, are not implemented or tested.
No runtime trace, model check, or flake reproduction has been performed.
Configuration round-trip tests are not evidence of lifecycle correctness.

Next steps:

1. Review TS-001 through TS-006 and decide domain semantics, preference authority,
   and invalid-input policy.
2. Resolve the build configuration blocker and run `test_transport_selection`.
   Then define and test the transport-specific compatibility facts before
   connecting the prototype to the publisher.
3. Specify registration transitions using explicit events and failure cases.
   Test repeated registration, changed capabilities, removal, and shutdown
   without sleeps used as readiness conditions.
4. Implement the accepted simplification and record intentional differences.