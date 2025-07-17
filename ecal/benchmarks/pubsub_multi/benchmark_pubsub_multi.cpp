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

#define REGISTRATION_DELAY_MS   2000
#define WARMUP_TIME_S           2

#define BACKGROUND_TOPIC_COUNT_MIN            1
#define BACKGROUND_TOPIC_COUNT_MAX            32
#define BACKGROUND_TOPIC_COUNT_MULTIPLIER     2
#define PER_TOPIC_RANGE_START       1
#define PER_TOPIC_RANGE_LIMIT       1<<24
#define PER_TOPIC_RANGE_MULTIPLIER  1<<12


/*
 *
 * Benchmarking the eCAL send process with multiple topics (background load)
 * 
*/
namespace Multi_Send {
  // Define kill signal variable
  std::atomic_bool atom_stop = false;

  // Benchmark function
  static void BM_eCAL_Multi_Send(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    size_t payload_size = state.range(1);
    std::vector<char> content_vector(payload_size);
    char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create the background publishers, count depends on first argument
    int topic_count = state.range(0);
    std::vector<eCAL::CPublisher> background_publisher_vector;
    for (int i=0; i<topic_count; i++) {
      background_publisher_vector.emplace_back(eCAL::CPublisher("background_topic_" + std::to_string(i)));
    }

    // Create background subscribers in a new thread
    std::thread background_receiver_thread([topic_count](){
      // Create the subscribers
      std::vector<eCAL::CSubscriber> background_subscriber_vector;
      for (int i=0; i<topic_count; i++) {
        background_subscriber_vector.emplace_back(eCAL::CSubscriber("background_topic_" + std::to_string(i)));
      }
      // Keep this thread alive
      while(!atom_stop) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    });

    // Reset kill signal
    atom_stop = false;
    
    // Create background publishers in new threads
    std::vector<std::thread> background_thread_vector;
    for (int i=0; i<topic_count; i++) {
      background_thread_vector.emplace_back([i, payload_size](){
        // Create own payload to send
        std::vector<char> content_vector(payload_size);

        // Create publisher
        eCAL::CPublisher background_publisher("background_topic_" + std::to_string(i));

        // Wait for eCAL synchronization
        std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

        // Keep sending
        while (!atom_stop) {
          background_publisher.Send(content_vector.data(), payload_size);
        }
      });
    }
    
    // Create publisher for the main thread
    eCAL::CPublisher publisher("benchmark_topic");

    // Create main subscriber in a new thread
    std::thread main_receiver_thread([](){
      eCAL::CSubscriber subscriber("benchmark_topic");
      // Keep this thread alive
      while(!atom_stop) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    });

    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload in the main thread
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Send kill signal to all threads
    atom_stop = true;

    // Wait for threads to finish and finalize eCAL
    background_receiver_thread.join();
    main_receiver_thread.join();
    for (auto& t : background_thread_vector) {
      t.join();
    }
    eCAL::Finalize();
  }

  // Register benchmark
  BENCHMARK(BM_eCAL_Multi_Send)
    ->ArgsProduct({
      benchmark::CreateRange(BACKGROUND_TOPIC_COUNT_MIN, BACKGROUND_TOPIC_COUNT_MAX, BACKGROUND_TOPIC_COUNT_MULTIPLIER),
      benchmark::CreateRange(PER_TOPIC_RANGE_START, PER_TOPIC_RANGE_LIMIT, PER_TOPIC_RANGE_MULTIPLIER)})
    ->UseRealTime()
    ->MinWarmUpTime(WARMUP_TIME_S);
}

// Benchmark execution
BENCHMARK_MAIN();