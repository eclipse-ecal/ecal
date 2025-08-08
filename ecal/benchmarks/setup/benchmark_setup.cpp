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

#include <thread>


constexpr int minimum_time_s = 5;


/*
 *
 * Benchmarking the eCAL initialization
 * 
*/
namespace Initialize {
   void BM_eCAL_Initialize(benchmark::State& state) { 
      // This is the benchmarked section: Initializing eCAL
      for (auto _ : state) {
         eCAL::Initialize("Benchmark");
      }
 
      // Finalize eCAL
      eCAL::Finalize();
   }
   // Register the benchmark function
   BENCHMARK(BM_eCAL_Initialize);
}


/*
 *
 * Benchmarking the eCAL initialization and finalization
 * 
*/
namespace Initialize_and_Finalize {
   void BM_eCAL_Initialize_and_Finalize(benchmark::State& state) {
      // This is the benchmarked section: Initializing and Finalizing eCAL
      for (auto _ : state) {
         eCAL::Initialize("Benchmark");
         eCAL::Finalize();
      }
   }
   // Register the benchmark function
   BENCHMARK(BM_eCAL_Initialize_and_Finalize);
}


/*
 *
 * Benchmarking the eCAL publisher creation process
 * 
*/
namespace Publisher_Creation {
   void BM_eCAL_Publisher_Creation(benchmark::State& state) {
      // Initialize eCAL
      eCAL::Initialize("Benchmark");

      // This is the benchmarked section: Creating a publisher
      for (auto _ : state) {
         eCAL::CPublisher publisher("benchmark_topic");
      }

      // Finalize eCAL
      eCAL::Finalize();
   }
   // Register the benchmark function
   BENCHMARK(BM_eCAL_Publisher_Creation);
}


/*
 *
 * Benchmarking the eCAL subscriber creation process
 * 
*/
namespace Subscriber_Creation {
   void BM_eCAL_Subscriber_Creation(benchmark::State& state) {
      // Initialize eCAL
      eCAL::Initialize("Benchmark");

      // This is the benchmarked section: Creating a subscriber
      for (auto _ : state) {
         eCAL::CSubscriber subscriber("benchmark_topic");
      }

      // Finalize eCAL
      eCAL::Finalize();
   }
   // Register the benchmark function
   BENCHMARK(BM_eCAL_Subscriber_Creation);
}


/*
 *
 * Benchmarking the eCAL registration delay
 * 
*/
namespace Registration_Delay {
   void BM_eCAL_Registration_Delay(benchmark::State& state) {
      // Initialize eCAL
      eCAL::Initialize("Benchmark");     

      // This is the benchmarked section: Creating publisher and subscriber (untimed) and waiting until the publisher is subscribed
      for (auto _ : state) {
         state.PauseTiming();
         eCAL::CPublisher publisher("benchmark_topic");
         eCAL::CSubscriber subscriber("benchmark_topic");
         state.ResumeTiming();

         while (publisher.GetSubscriberCount() == 0) { std::this_thread::yield(); }
      }

      // Finalize eCAL
      eCAL::Finalize();
   }
   BENCHMARK(BM_eCAL_Registration_Delay)->MinTime(minimum_time_s);
}


// Benchmark execution
BENCHMARK_MAIN();