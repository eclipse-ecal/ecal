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
#include <util/ecal_expmap.h>
#include <ecal_struct_sample_registration.h>

#include <benchmark/benchmark.h>

#include <unordered_set>
#include <random>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>
#include <unordered_map>


using namespace eCAL;

namespace
{

  Registration::Sample MakeRegPublisherSample(
    uint64_t entity_id,
    uint32_t process_id,
    const std::string& host,
    const std::string& topic_name,
    const std::string& dtype_name,
    const std::string& dtype_desc)
  {
    Registration::Sample s{};
    s.cmd_type = bct_unreg_publisher;
    s.identifier.entity_id = entity_id;
    s.identifier.process_id = process_id;
    s.identifier.host_name = host;
    s.topic.topic_name = topic_name;
    s.topic.datatype_information.name = dtype_name;
    s.topic.datatype_information.descriptor = dtype_desc;
    return s;
  }

  std::vector<uint64_t> random_uint64_vector(std::size_t N)
  {
    std::vector<uint64_t> result;
    result.reserve(N);

    std::unordered_set<uint64_t> used;
    used.reserve(N * 2);  // reduce rehashing

    // Random engine
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    while (result.size() < N) {
      uint64_t x = dist(gen);
      if (used.insert(x).second) {  // true = inserted successfully (not duplicate)
        result.push_back(x);
      }
    }

    return result;
  }

  std::vector<Registration::Sample> MakeUniquePublisherSamples(std::size_t n,
    uint32_t pid = 4242,
    const std::string& host = "bench-host")
  {
    std::vector<Registration::Sample> v;
    v.reserve(n);
    auto ids = random_uint64_vector(n);

    for (std::size_t i = 0; i < n; ++i)
    {
      const uint64_t eid = ids[i];
      const std::string topic = "topic_" + std::to_string(i);
      const std::string dtype = "type_" + std::to_string(i % 8);
      const std::string desc = "desc_" + std::to_string(i % 4);
      v.emplace_back(MakeRegPublisherSample(eid, pid, host, topic, dtype, desc));
    }
    return v;
  }

  void Shuffle(std::vector<Registration::Sample>& v, uint64_t seed = 42)
  {
    std::mt19937_64 rng{ seed };
    std::shuffle(v.begin(), v.end(), rng);
  }

  /**
   * Benchmark: insertion of N elements into an empty cache
   */
  template <typename ExpiredMap>
  void BM_CExpirationMap_Insert(benchmark::State& state)
  {
    const std::size_t N = static_cast<std::size_t>(state.range(0));

    // Created once per benchmark run, not measured
    const auto samples = MakeUniquePublisherSamples(N);

    for (auto _ : state)
    {
      ExpiredMap cache;

      // Measure inserting N elements into an empty cache
      for (const auto& sample : samples)
      {
        cache.insert({sample.identifier, sample});
      }

      benchmark::DoNotOptimize(cache);
    }

    state.SetItemsProcessed(state.iterations() * N);
  }

  /**
   * Benchmark: update (access) of existing elements in a cache of size N
   *
   * This exercises operator[] which in your class also updates the timestamp.
   */
  template <typename ExpiredMap>
  static void BM_CExpirationMap_Find(benchmark::State& state)
  {
    const std::size_t N = static_cast<std::size_t>(state.range(0));

    const auto samples = MakeUniquePublisherSamples(N);

    ExpiredMap cache;
    for (const auto& sample : samples)
    {
      cache.insert({ sample.identifier, sample });
    }

    for (auto _ : state)
    {
      for (int i = 0; i < N; ++i)
      {
        const auto& index_to_update = samples[i].identifier;
        const auto& cache_iterator = cache.find(index_to_update);
      }
      benchmark::DoNotOptimize(cache);
    }

    state.SetItemsProcessed(state.iterations()*N);
  }

  /**
 * Benchmark: update (access) of existing elements in a cache of size N
 *
 * This exercises operator[] which in your class also updates the timestamp.
 */
  template <typename ExpiredMap>
  static void BM_CExpirationMap_Update(benchmark::State& state)
  {
    const std::size_t N = static_cast<std::size_t>(state.range(0));

    const auto samples = MakeUniquePublisherSamples(N);

    ExpiredMap cache;
    for (const auto& sample : samples)
    {
      cache.insert({ sample.identifier, sample });
    }

    for (auto _ : state)
    {
      for (int i = 0; i < N; ++i)
      {
        const auto& index_to_update = samples[i].identifier;
        const auto& cache_iterator = cache.find(index_to_update);

        cache.update(cache_iterator);
      }
      benchmark::DoNotOptimize(cache);
    }

    state.SetItemsProcessed(state.iterations() * N);
  }


  /**
   * Benchmark: erase N elements from a cache that currently holds N elements
   */
  template <typename ExpiredMap>
  static void BM_CExpirationMap_Erase(benchmark::State& state)
  {
    const std::size_t N = static_cast<std::size_t>(state.range(0));

    const auto samples = MakeUniquePublisherSamples(N);

    ExpiredMap cache;
    for (const auto& sample : samples)
    {
      cache.insert({ sample.identifier, sample });
    }


    for (auto _ : state)
    {

      // Erase all elements by key
      for (std::size_t i = 0; i < N; ++i)
      {
        auto erased = cache.erase(samples[i].identifier);
        benchmark::DoNotOptimize(erased);
      }

      benchmark::DoNotOptimize(cache);
    }

    state.SetItemsProcessed(state.iterations() * N);
  }

  // ------------------------- Registration -------------------------
  using SampleTrackerMap = Util::CExpirationMap<Registration::SampleIdentifier, Registration::Sample, std::chrono::steady_clock, std::map>;
  using SampleTrackerUnorderedMap = Util::CExpirationMap<Registration::SampleIdentifier, Registration::Sample, std::chrono::steady_clock, std::unordered_map>;


  // Adjust the argument list to your dataset sizes
  template<class ExpiredMap>
  void RegisterFamily(const char* tag)
  {
    benchmark::RegisterBenchmark(
      std::string("ExpiredMap/Insert/") + tag,
      &BM_CExpirationMap_Insert<ExpiredMap>)
      ->Arg(100)
      ->Arg(1'000)
      ->Arg(10'000)
      ->Arg(100'000);

    benchmark::RegisterBenchmark(
      std::string("ExpiredMap/Find/") + tag,
      &BM_CExpirationMap_Find<ExpiredMap>)
      ->Arg(100)
      ->Arg(1'000)
      ->Arg(10'000)
      ->Arg(100'000);

    benchmark::RegisterBenchmark(
      std::string("ExpiredMap/Update/") + tag,
      &BM_CExpirationMap_Update<ExpiredMap>)
      ->Arg(100)
      ->Arg(1'000)
      ->Arg(10'000)
      ->Arg(100'000);

    benchmark::RegisterBenchmark(
      std::string("ExpiredMap/Remove/") + tag,
      &BM_CExpirationMap_Erase<ExpiredMap>)
      ->Arg(100)
      ->Arg(1'000)
      ->Arg(10'000)
      ->Arg(100'000);
  }
}

// Register both variants
int main(int argc, char** argv)
{
  ::benchmark::Initialize(&argc, argv);

  RegisterFamily<SampleTrackerMap>("ExpiredMap using std::map");
  RegisterFamily<SampleTrackerUnorderedMap>("ExpiredMap using std::unordered_map");
  // We can potentially register other implementations to compare the benchmarks.

  ::benchmark::RunSpecifiedBenchmarks();
}