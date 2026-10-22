# eCAL core specification workbench

Status: working draft. User-confirmed design directions are recorded separately
from proposed contracts; implementation and conformance remain outstanding.

## Purpose and priorities

Recover enough of the current system to design a simpler replacement, not to
reproduce its implementation. Prefer straightforward architecture, explicit
state and ownership, and predictable behavior over historical compatibility.
Intentional behavior changes are allowed when their impact is documented and
reviewed. Do not preserve a race, incidental delay, or flaky test as a contract.

Use these priorities when evaluating a design:

1. Clear, useful observable guarantees and correct resource ownership.
2. Few states, transitions, special cases, and independent sources of truth.
3. Deterministic decisions and explicit synchronization.
4. Testability without arbitrary sleeps or scheduler assumptions.
5. Compatibility and performance within explicitly agreed constraints.

Simplicity does not mean omitting failure handling. Describe what happens when
a peer disappears, an operation fails, or shutdown overlaps ongoing work.
Record performance requirements as measurable budgets rather than assumptions.

## Confirmed design directions

Recorded from maintainer discussion on 2026-10-21:

- The selected connection method stays fixed between two peer endpoints for
  the lifetime of their connection. Runtime transport switching is not a goal.
- Prefer a defined transport preference order and a separate pairwise eligibility
  calculation over independent local/remote priority policies.
- Advertised capabilities and transport-domain compatibility are primary inputs;
  duplicating peer capabilities in user configuration is not the default design.

Transport-derived readiness and moving recurring availability checks from
discovery into transports are candidates, not yet agreed mechanisms. See
[peer-connection-lifecycle.md](peer-connection-lifecycle.md) for limits and
open questions. These directions do not accept every proposed MUST below.

## Specification levels

| Level | Question | Artifact |
| --- | --- | --- |
| System | What does eCAL promise its users? | Vocabulary, guarantees, quality requirements |
| Subsystem | Who owns each responsibility and resource? | Boundaries and dependency diagrams |
| Interface | What may callers rely on? | Preconditions, outcomes, errors, ownership, concurrency |
| Protocol/state | What transitions and ordering are legal? | Decision tables, state machines, selective formal models |
| Conformance | How can a claim be disproved? | Cases, properties, fault tests, measurements |

Do not mirror every C++ class in the specification. Document internal structure
only where it explains a boundary, constraint, or simplification opportunity.
Existing architecture diagrams live in [../arch/Readme.md](../arch/Readme.md);
record accepted design decisions in `../adr/`.
This Markdown workbench is not currently integrated into the Sphinx build.

## Evidence and authority

Each feature document separates:

- **Recovered behavior:** claims about a pinned baseline, with source/test/trace
  evidence. Label claims source-derived, runtime-observed, inferred, or unknown.
- **Proposed target:** normative MUST/SHOULD/MAY statements with stable IDs.
  These become authoritative only after maintainer acceptance.
- **Gaps and decisions:** intentional differences, unknowns, and alternatives.
- **Conformance:** checks linked to requirement IDs and an explicit coverage
  status. A model passing its own examples does not validate the implementation.

For each contract, state inputs, outcome, failures, ownership, concurrency,
timing, verification, and compatibility impact, or explicitly mark them outside
the slice. Distinguish safety (nothing invalid happens) from liveness (progress
eventually happens under stated assumptions).

## Working method

1. Pin the baseline and inspect one controlling path and nearby tests.
2. Recover the minimum behavior needed to identify useful contracts and risks.
3. Propose a smaller model; list removed states and changed behavior explicitly.
4. Review the target before implementing behavior changes.
5. Test decisions with pure inputs; test transitions with controlled events.
6. Replace behind the boundary and validate against accepted contracts.

For timing-sensitive tests, prefer observable readiness and completion signals,
injected clocks for expiry logic, and controlled execution where practical.
Use bounded waits as failure guards, not as evidence that an operation is ready.
Keep real-process/network tests for integration properties; do not require exact
callback ordering unless the contract actually promises it. Investigate flakes
instead of hiding them with retries or longer sleeps.

Agents should produce evidence and counterexamples, not invent guarantees.
Have an independent review challenge both the recovered claims and the test
oracle. Leave unresolved policy choices visible for maintainer decisions.

## Pilot

Start with [publisher-transport-selection.md](publisher-transport-selection.md).
Its [transport-selection-cases.json](transport-selection-cases.json) contains
decision examples, not an implementation conformance test. Next, establish a
focused C++ test boundary before replacing the selector or connection lifecycle.
An isolated selector prototype and exhaustive C++ tests now exist; see the pilot
for API limits and validation status. They do not change publisher behavior.
The adjacent [peer-connection-lifecycle.md](peer-connection-lifecycle.md) records
fixed selection, proposed readiness events, and the discovery/liveness boundary.