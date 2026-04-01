# Tracing Test Summary

**File:** `ecal/tests/cpp/tracing_test/src/tracing_test.cpp`
**Total:** 69 tests across 10 test suites

---

## TracingTypesTest (8 tests)

| Test | What it validates |
|---|---|
| `TracingVersionConstant` | `kTracingVersion` equals `"1.0.0"` |
| `DefaultTracingBatchSize` | `kDefaultTracingBatchSize` equals `10` |
| `SSpanDataDefaultInitialization` | All `SSpanData` fields are zero after value-initialization |
| `SSpanDataAssignment` | All `SSpanData` fields can be assigned and read back correctly |
| `STopicMetadataDefaultVersion` | `STopicMetadata::tracing_version` defaults to `kTracingVersion` |
| `STopicMetadataAssignment` | All `STopicMetadata` fields can be assigned and read back correctly |
| `OperationTypeEnum` | `send=0`, `receive=1`, `callback_execution=2` |
| `TopicDirectionEnum` | `publisher=0`, `subscriber=1` |

## TracingLayerTypeTest (8 tests)

| Test | What it validates |
|---|---|
| `LayerTypeValues` | Enum values: `none=0`, `shm=1`, `udp=2`, `tcp=4`, combined values `3,5,6,7` |
| `LayerTypeBitwise` | Layer types combine correctly with bitwise OR |
| `ToTracingLayerTypeSHM` | `toTracingLayerType(tl_ecal_shm)` → `tl_trace_shm` |
| `ToTracingLayerTypeUDP` | `toTracingLayerType(tl_ecal_udp)` → `tl_trace_udp` |
| `ToTracingLayerTypeTCP` | `toTracingLayerType(tl_ecal_tcp)` → `tl_trace_tcp` |
| `ToTracingLayerTypeAll` | `toTracingLayerType(tl_all)` → `tl_trace_all` |
| `ToTracingLayerTypeUnknown` | Invalid `eTLayerType(999)` → `tl_trace_none` |
| `ToTracingLayerTypeNone` | `toTracingLayerType(tl_none)` → `tl_trace_none` |

## TracingProviderTest (11 tests)

| Test | What it validates |
|---|---|
| `SingletonInstance` | `getInstance()` returns the same address on repeated calls |
| `DefaultBatchSize` | Buffering 9 spans doesn't trigger auto-flush (batch size = 10) |
| `SetBatchSizeAffectsAutoFlush` | Changing batch size to 3 triggers flush on the 3rd span |
| `BufferSpan` | A single span is buffered with correct field values |
| `BufferMultipleSpans` | 5 spans are buffered in order with correct entity IDs |
| `FlushSpans` | `flushSpans()` empties the buffer |
| `FlushEmptyBuffer` | Flushing an empty buffer doesn't throw |
| `AutoFlushOnBatchSize` | Buffer empties automatically when batch size is reached |
| `AutoFlushMultipleBatches` | Multiple auto-flush cycles work correctly, residual spans stay buffered |
| `AddTopicMetadata` | Metadata is written to JSONL file with all fields (publisher direction) |
| `AddTopicMetadataSubscriber` | Metadata with subscriber direction is written correctly |

## SpanTest (7 tests)

| Test | What it validates |
|---|---|
| `SendSpanConstruction` | RAII `CSpan` (send) buffers correct entity_id, process_id, payload_size, clock, layer, op_type; topic_id=0 for send |
| `SendSpanTimestamps` | `start_ns < end_ns`, both > 0, duration ≥ 0.5ms (with 1ms sleep) |
| `ReceiveSpanConstruction` | RAII `CSpan` (receive) buffers correct entity_id, topic_id, process_id, payload_size, clock, layer, op_type |
| `ReceiveSpanTimestamps` | Receive span has valid start/end timestamps with `start < end` |
| `CallbackExecutionSpan` | `CSpan` with `callback_execution` op_type records correctly |
| `MultipleOperationTypes` | Two spans with different op_types are buffered in order |
| `MultipleLayerTypes` | Four spans with SHM, UDP, TCP, SHM+UDP layers are buffered correctly |

## TracingWriterTest (11 tests)

| Test | What it validates |
|---|---|
| `WriterConstruction` | `CTracingWriter` constructs without throwing |
| `WriteBatchSpansVerifyContent` | 3-span batch written to JSONL; all 9 fields verified per line |
| `WriteBatchSpansReceive` | Receive span written with correct op_type, topic_id, layer |
| `WriteBatchSpansAppends` | Two sequential writes append (2 lines in file, not 1) |
| `WriteEmptyBatch` | Empty batch produces no output (file empty or absent) |
| `WriteTopicMetadataVerifyContent` | Metadata JSONL contains all 8 fields (publisher direction) |
| `WriteTopicMetadataSubscriber` | Subscriber direction is serialized as `"subscriber"` |
| `WriteMultipleMetadataAppends` | 3 metadata writes append correctly with alternating directions |
| `EmptyStringMetadataFields` | Empty strings and zero IDs are written as `""` / `0` |
| `WriteBatchSpansCallbackExecution` | `callback_execution` op_type and TCP layer written correctly |
| `WriteBatchSpansAllLayerTypes` | All 8 layer type values (none through all) written and read back |

## TracingIntegrationTest (4 tests)

| Test | What it validates |
|---|---|
| `SpanBufferingAndFlushing` | CSpan → buffer → getSpans → flushSpans → empty buffer |
| `MixedSendAndReceiveSpans` | Send and receive spans coexist in buffer with correct fields |
| `CompleteTracingFlow` | Full flow: addTopicMetadata → CSpan creation → buffer verify → flush → JSONL file verify |
| `AutoFlushWritesToFile` | Auto-flush (batch size 3) writes spans to JSONL file and empties buffer |

## ThreadSafetyTest (5 tests)

| Test | What it validates |
|---|---|
| `ConcurrentSpanBuffering` | 4 threads × 25 CSpan RAII objects → exactly 100 spans buffered (no data loss) |
| `ConcurrentMetadataWriting` | 4 threads × 5 metadata writes → 20 valid JSONL lines (no corruption) |
| `ConcurrentBufferingWithAutoFlush` | 4 threads × 20 spans with batch size 5 → all 80 spans written to file |
| `ConcurrentGetSpans` | 3 reader threads + 1 writer thread → no crashes, correct final count (100 spans) |
| `ConcurrentWriteBatchSpansIntegrity` | **Regression:** 8 threads × 20 batches × 5 spans → 800 valid JSONL lines (tests `spans_mutex_` fix) |

## EdgeCaseTest (7 tests)

| Test | What it validates |
|---|---|
| `AllLayerTypesCombined` | CSpan with `tl_trace_all` (7) stores correct layer value |
| `NoneLayerType` | CSpan with `tl_trace_none` (0) and payload 0 stores correctly |
| `EmptyStringMetadata` | Empty-string metadata fields are written without error |
| `BatchSizeOfOne` | Batch size = 1 triggers immediate auto-flush on every span |
| `RapidFlushCycles` | 10 rapid buffer-then-flush cycles work correctly |
| `MaxEntityAndProcessIds` | `UINT64_MAX` for entity_id, process_id, topic_id written and read back correctly |
| `ReceiveSpanViaProviderFlush` | Full path: CSpan receive → buffer → verify fields → flush → verify JSONL output |

## ScaleTest (5 tests)

| Test | What it validates |
|---|---|
| `HighFanoutProducers` | 500 threads × 200 spans via `bufferSpan()` → 100,000 spans, zero data loss |
| `HighFanoutViaSpanRAII` | 200 threads × 100 CSpan RAII objects → 20,000 spans written to file |
| `MixedPubSubMetadataAndSpans` | 200 threads each registering metadata + 50 spans → 200 metadata + 10,000 spans |
| `BatchSizeSweep` | Same load tested at batch sizes 1, 5, 10, 50, 100, 500 — correctness is batch-size-independent |
| `SustainedBurst` | 100 threads × 500 spans with batch size 25 — stresses mutex + file I/O under sustained load |

## PubSubStressTest (3 tests)

| Test | What it validates |
|---|---|
| `TwoPubTwoSubHighFrequency` | 2 real publishers + 2 subscribers, 500 messages each (concurrent) → exactly 1,000 send spans; valid timestamps on all spans |
| `MultiTopicHighFrequency` | 3 publishers on 3 topics, 300 messages each (concurrent) → 900 send spans; all use SHM layer |
| `HighFrequencyWithSlowCallback` | 1 pub + 1 sub, 200 messages, 50µs callback delay → 200 send spans; callback_execution spans have duration ≥ 40µs |

---

## Bug Fixes Validated by These Tests

| Bug | Fix | Regression test |
|---|---|---|
| Data race in `bufferSpan()` — `span_buffer_.size()` read outside lock | Moved size check inside lock, use local `should_flush` flag | `ConcurrentBufferingWithAutoFlush`, `HighFanoutProducers` |
| Thread-unsafe `getSpans()` — returned buffer without holding mutex | Added `lock_guard` before returning `span_buffer_` | `ConcurrentGetSpans` |
| No mutex on `writeBatchSpans()` — concurrent flushes produced corrupted JSONL | Added `spans_mutex_` to `CTracingWriter` | `ConcurrentWriteBatchSpansIntegrity` |
