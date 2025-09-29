/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#include <ecal/ecal.h>
#include <ecal_descgate.h>

#include <benchmark/benchmark.h>


#include <random>
#include <thread>
#include <mutex>
#include <condition_variable>


using namespace eCAL;

static Registration::Sample MakeRegPublisherSample(
    uint64_t entity_id,
    uint32_t process_id,
    const std::string& host,
    const std::string& topic_name,
    const std::string& dtype_name,
    const std::string& dtype_desc)
{
  Registration::Sample s{};
  s.cmd_type                  = bct_reg_publisher;
  s.identifier.entity_id      = entity_id;
  s.identifier.process_id     = process_id;
  s.identifier.host_name      = host;
  s.topic.topic_name          = topic_name;
  s.topic.datatype_information.name       = dtype_name;
  s.topic.datatype_information.descriptor = dtype_desc;
  return s;
}

static Registration::Sample MakeUnregPublisherSample(const Registration::Sample& reg)
{
  Registration::Sample s = reg;
  s.cmd_type = bct_unreg_publisher;
  return s;
}

static std::vector<Registration::Sample> MakeUniquePublisherSamples(std::size_t n,
                                                                    uint32_t pid = 4242,
                                                                    const std::string& host = "bench-host")
{
  std::vector<Registration::Sample> v;
  v.reserve(n);
  for (std::size_t i = 0; i < n; ++i)
  {
    const uint64_t eid = static_cast<uint64_t>(i + 1);
    const std::string topic = "topic_" + std::to_string(i);
    const std::string dtype = "type_"  + std::to_string(i % 8);
    const std::string desc  = "desc_"  + std::to_string(i % 4);
    v.emplace_back(MakeRegPublisherSample(eid, pid, host, topic, dtype, desc));
  }
  return v;
}

static std::vector<Registration::Sample> MakeMatchingUnregSamples(const std::vector<Registration::Sample>& reg)
{
  std::vector<Registration::Sample> u;
  u.reserve(reg.size());
  for (const auto& s : reg) u.emplace_back(MakeUnregPublisherSample(s));
  return u;
}

static void Shuffle(std::vector<Registration::Sample>& v, uint64_t seed = 42)
{
  std::mt19937_64 rng{seed};
  std::shuffle(v.begin(), v.end(), rng);
}

// Cold insert: Apply N unique registration samples into a fresh gate
template<class GateT>
static void BM_ApplySample_ColdInsert(benchmark::State& state)
{
  const std::size_t N = static_cast<std::size_t>(state.range(0));

  std::vector<Registration::Sample> reg_samples;

  for (auto _ : state)
  {
    state.PauseTiming();
    GateT gate;  // new gate every iteration (cold)
    reg_samples = MakeUniquePublisherSamples(N);
    Shuffle(reg_samples, 123);
    state.ResumeTiming();

    for (const auto& s : reg_samples)
      gate.ApplySample(s, static_cast<eTLayerType>(0));
  }

  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(N));
}

// No-change reapply: Pre-populate once per iteration, time the second pass only
template<class GateT>
static void BM_ApplySample_ReapplyNoChange(benchmark::State& state)
{
  const std::size_t N = static_cast<std::size_t>(state.range(0));
  std::vector<Registration::Sample> reg_samples;

  for (auto _ : state)
  {
    state.PauseTiming();
    GateT gate;
    reg_samples = MakeUniquePublisherSamples(N);
    Shuffle(reg_samples, 456);
    // Populate once (not timed)
    for (const auto& s : reg_samples)
      gate.ApplySample(s, static_cast<eTLayerType>(0));
    state.ResumeTiming();

    // Re-apply same samples (should take the no-change fast path)
    for (const auto& s : reg_samples)
      gate.ApplySample(s, static_cast<eTLayerType>(0));
  }

  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(N));
}

// Unregister: Pre-populate once, then time unregister pass
template<class GateT>
static void BM_ApplySample_Unregister(benchmark::State& state)
{
  const std::size_t N = static_cast<std::size_t>(state.range(0));
  std::vector<Registration::Sample> reg_samples;
  std::vector<Registration::Sample> unreg_samples;

  for (auto _ : state)
  {
    state.PauseTiming();
    GateT gate;
    reg_samples = MakeUniquePublisherSamples(N);
    Shuffle(reg_samples, 789);
    unreg_samples = MakeMatchingUnregSamples(reg_samples);

    // Populate once (not timed)
    for (const auto& s : reg_samples)
      gate.ApplySample(s, static_cast<eTLayerType>(0));
    state.ResumeTiming();

    // Time unregister
    for (const auto& s : unreg_samples)
      gate.ApplySample(s, static_cast<eTLayerType>(0));
  }

  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(N));
}

// ------------------------- Registration -------------------------

// Adjust the argument list to your dataset sizes
template<class GateT>
void RegisterFamily(const char* tag)
{
  benchmark::RegisterBenchmark(
    std::string("ApplySample/ColdInsert/") + tag,
    &BM_ApplySample_ColdInsert<GateT>)
    ->Arg(1'000)
    ->Arg(100'000)
    ->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark(
    std::string("ApplySample/ReapplyNoChange/") + tag,
    &BM_ApplySample_ReapplyNoChange<GateT>)
    ->Arg(1'000)
    ->Arg(100'000)
    ->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark(
    std::string("ApplySample/Unregister/") + tag,
    &BM_ApplySample_Unregister<GateT>)
    ->Arg(1'000)
    ->Arg(100'000)
    ->Unit(benchmark::kMillisecond);
}

// Register both variants
int main(int argc, char** argv)
{
  ::benchmark::Initialize(&argc, argv);

  RegisterFamily<CDescGate>("New");
  // We can potentially register other implementations to compare the benchmarks.

  ::benchmark::RunSpecifiedBenchmarks();
}