# Peer connection lifecycle

Status: design directions and proposed model, not an implemented protocol.
Discussion baseline: 2026-10-21. Source baseline and selection evidence are in
[publisher-transport-selection.md](publisher-transport-selection.md).

## Decisions and evidence

**User-confirmed direction:** the connection method stays fixed between two
peers. Capabilities and transport-specific eligibility determine available
methods; a defined preference order chooses among them.

For this draft, a connection is one publisher/subscriber endpoint relationship,
not every topic relationship between two processes. Endpoint identity,
incarnation/restart detection, and whether process-wide selection is desired
still need agreement. Resource sharing across connections is permitted.

**Source-derived observation:** `ApplySubscriberRegistration` inserts a pending
connection and promotes it on a subsequent registration. This is not an explicit
transport-readiness acknowledgement. It does not establish that delays are the
only readiness mechanism anywhere in eCAL.

**User-reported operational context:** readiness currently relies on delays;
recurring discovery supports diagnostics such as continued peer availability.
These reports motivate this proposal but have not been reproduced in this pilot.

## Candidate separation of responsibilities

| Responsibility | Candidate owner | Must not imply |
| --- | --- | --- |
| Find endpoints and obtain capabilities/domain descriptors | Discovery | Data path ready |
| Compute eligibility and select once | Connection creation | Transport setup succeeded |
| Establish and report local send/receive prerequisites | Selected transport adapter | Remote delivery or peer health forever |
| Report loss of readiness and release resources | Transport adapter and connection owner | Automatic transport switching |
| Diagnose availability | Discovery lease or explicit transport-specific mechanism | Certain failure detection in an asynchronous network |

Eligibility is a compatibility decision from advertised facts. Readiness is a
runtime condition from the selected transport. Liveness is an observation with
a timeout/failure-detection policy, not a consequence of either one.

Do not require every transport to create a physical peer-to-peer channel.
The connection can be a logical relationship over multicast or shared resources.

## Proposed minimal state model

Discovery owns candidate records. Only an eligible pair creates a connection;
there is no connection object in a waiting-for-discovery state. Selection is
committed on creation and retained for that connection's lifetime.

| State | Event | Outcome |
| --- | --- | --- |
| Establishing | Selected adapter reports required setup complete | Ready; emit readiness notification once |
| Establishing | Setup fails, is cancelled, or reaches its explicit deadline | Closed with reason; release resources |
| Ready | Identical discovery refresh | No lifecycle transition; diagnostics may refresh |
| Ready | Transport reports readiness lost, or explicit removal/shutdown occurs | Closed with reason; release resources |
| Closed | Delayed refresh or stale adapter completion | No revival; no readiness notification |

Repeated unchanged discovery announcements in Establishing also do not advance
readiness. Ready describes the adapter's stated local prerequisites, not a
delivery guarantee; that definition must be published per transport.
Serialize lifecycle events under one owner; no specific threading mechanism is
chosen yet. Associate async completions with a connection incarnation so stale
events cannot affect another connection using the same endpoint names.

Retries and reconnects are deliberately not modeled as extra states yet.
Decide their policy before implementation. In particular, closing and recreating
a connection MUST NOT become an implicit way to switch transports between the
same live endpoint pair. Capability withdrawal can invalidate a connection,
but cannot reselect it. Restart/identity semantics determine when a new choice
is legitimate.

## Readiness and liveness limits

- TCP socket establishment can contribute to readiness, but may not establish
  application-level subscription readiness or timely detection of peer death.
- SHM readiness needs concrete resource and synchronization prerequisites;
  domain compatibility alone does not prove them. Crash detection needs a
  separate mechanism even when no data is being sent.
- UDP/multicast has no inherent peer connection or acknowledgement. Local socket
  setup can report local readiness, not remote receiver readiness. Stronger
  readiness requires an explicit protocol, otherwise expose the weaker meaning.

Moving availability checks into transports is an **open architectural option**.
First separate discovery refresh needs: finding late joiners, recovering lost
announcements, refreshing capability/diagnostic data, and detecting stale peers.
Transport health cannot automatically replace all four. A discovery lease may
remain simpler than separate heartbeat protocols for every transport.

## Proposed checks and remaining decisions

No executable lifecycle model or implementation test exists for this draft.
Use controlled adapter events and an injected clock to check:

1. Any number of discovery refreshes cannot make a connection Ready.
2. Matching transport setup completion makes it Ready exactly once.
3. Capability/preference updates cannot change a committed transport.
4. Setup failure, shutdown, and readiness loss close and clean up once.
5. A stale completion after closure cannot resurrect a connection.
6. A bounded setup deadline closes an Establishing connection without sleeps.

Before accepting this model, decide endpoint identity/incarnations, who owns the
preference order (publisher-owned is the simplest starting proposal), each
transport's domain and readiness semantics, readiness-loss reporting guarantees,
and reconnect/lease policy. Define public event/error surfaces only after those
semantics are agreed. No new handshake or heartbeat protocol is approved here.