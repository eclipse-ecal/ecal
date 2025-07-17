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
#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/subscriber.h>

#include <benchmark/benchmark.h>


#define REPETITIONS       10


/*
 *
 * Benchmarking the eCAL initialization
 * 
*/
namespace Initialize {
   static void BM_eCAL_Initialize(benchmark::State& state) { 
      // This is the benchmarked section: Initializing eCAL
      for (auto _ : state) {
         eCAL::Initialize("Benchmark");
      }
 
      // Finalize eCAL
      eCAL::Finalize();
   }
   // Register the benchmark function
   BENCHMARK(BM_eCAL_Initialize)->Repetitions(REPETITIONS);
}


/*
 *
 * Benchmarking the eCAL initialization and finalization
 * 
*/
namespace Initialize_and_Finalize {
   static void BM_eCAL_Initialize_and_Finalize(benchmark::State& state) {
      // This is the benchmarked section: Initializing and Finalizing eCAL
      for (auto _ : state) {
         eCAL::Initialize("Benchmark");
         eCAL::Finalize();
      }
   }
   // Register the benchmark function
   BENCHMARK(BM_eCAL_Initialize_and_Finalize)->Repetitions(REPETITIONS);
}


/*
 *
 * Benchmarking the eCAL publisher registration process
 * 
*/
namespace Publisher_Registration {
   static void BM_eCAL_Publisher_Registration(benchmark::State& state) {
      // Initialize eCAL
      eCAL::Initialize("Benchmark");

      // This is the benchmarked section: Registering a publisher
      for (auto _ : state) {
         eCAL::CPublisher publisher("benchmark_topic");
      }

      // Finalize eCAL
      eCAL::Finalize();
   }
   // Register the benchmark function
   BENCHMARK(BM_eCAL_Publisher_Registration)->Repetitions(REPETITIONS);
}


/*
 *
 * Benchmarking the eCAL subscriber registration process
 * 
*/
namespace Subscriber_Registration {
   static void BM_eCAL_Subscriber_Registration(benchmark::State& state) {
      // Initialize eCAL
      eCAL::Initialize("Benchmark");

      // This is the benchmarked section: Registering a subscriber
      for (auto _ : state) {
         eCAL::CSubscriber subscriber("benchmark_topic");
      }

      // Finalize eCAL
      eCAL::Finalize();
   }
   // Register the benchmark function
   BENCHMARK(BM_eCAL_Subscriber_Registration)->Repetitions(REPETITIONS);
}


// Benchmark execution
BENCHMARK_MAIN();