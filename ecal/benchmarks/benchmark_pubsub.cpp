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

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <benchmark/benchmark.h>

#define REGISTRATION_DELAY_MS   2000
#define RANGE_MULTIPLIER        1<<6
#define RANGE_START             1
#define RANGE_LIMIT             1<<18
#define WARMUP_TIME_S           2


/*
 *
 * Benchmarking the eCAL send process
 * 
*/
namespace Send {
  static void BM_eCAL_Send(benchmark::State& state) {
    // Create payload to send, size depends on current argument
    size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);

    // Initialize eCAL and create sender and receiver
    eCAL::Initialize("Benchmark");
    eCAL::CPublisher publisher("benchmark_topic");
    eCAL::CSubscriber subscriber("benchmark_topic");
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_vector.data(), payload_size);
    }

    // Finalize eCAL
    eCAL::Finalize();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send)->RangeMultiplier(RANGE_MULTIPLIER)->Range(RANGE_START, RANGE_LIMIT)->MinWarmUpTime(WARMUP_TIME_S)->Unit(benchmark::kMicrosecond);
}


/*
 *
 * Benchmarking the eCAL send and receive process
 * 
*/
namespace Send_and_Receive {
  // Define callback function to register incoming message
  std::mutex mtx;
  std::condition_variable convar;
  bool msg_received = false;

  void callback(){
    std::lock_guard<std::mutex> lock(mtx);
    msg_received = true;
    convar.notify_one();
    return;
  };

  static void BM_eCAL_Send_and_Receive(benchmark::State& state) {
    // Create payload to send, size depends on current argument
    size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);

    // Initialize eCAL, create sender and receiver and register callback function
    eCAL::Initialize("Benchmark");
    eCAL::CPublisher publisher("benchmark_topic");
    eCAL::CSubscriber subscriber("benchmark_topic");
    subscriber.SetReceiveCallback(std::bind(&callback));
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload and waiting for the receive callback
    for (auto _ : state) {
      msg_received = false;
      publisher.Send(content_vector.data(), payload_size);
      std::unique_lock<std::mutex> lock(mtx);
      convar.wait(lock, [] {return msg_received;});
    }

    // Finalize eCAL
    eCAL::Finalize();
  }
  BENCHMARK(BM_eCAL_Send_and_Receive)->RangeMultiplier(RANGE_MULTIPLIER)->Range(RANGE_START, RANGE_LIMIT)->Unit(benchmark::kMicrosecond);
}



/*
 * 
 * Benchmarking the eCAL receive latency (with manual timing)
 * 
*/
namespace Receive_Latency {
  // Define callback function to register incoming message and variables for manual timing
  std::mutex mtx;
  std::condition_variable convar;
  bool msg_received = false;
  std::chrono::high_resolution_clock::time_point time_start;
  std::chrono::high_resolution_clock::time_point time_end;

  void callback_manual(){
    time_end = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(mtx);
    msg_received = true;
    convar.notify_one();
    return;
  };

  static void BM_eCAL_Receive_Latency(benchmark::State& state) {
    // Create payload to send, size depends on current argument
    size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);

    // Initialize eCAL, create sender and receiver and register callback function
    eCAL::Initialize("Benchmark");
    eCAL::CPublisher publisher("benchmark_topic");
    eCAL::CSubscriber subscriber("benchmark_topic");
    subscriber.SetReceiveCallback(std::bind(&callback_manual));
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload (untimed) and waiting for the receive callback
    for (auto _ : state) {
      msg_received = false;
      publisher.Send(content_vector.data(), payload_size);
      std::unique_lock<std::mutex> lock(mtx);
      time_start = std::chrono::high_resolution_clock::now();
      convar.wait(lock, [&]() {return msg_received;});

      auto time_elapsed = std::chrono::duration_cast<std::chrono::duration<double>> (time_end - time_start);
      state.SetIterationTime(time_elapsed.count());
    }

    // Finalize eCAL
    eCAL::Finalize();
  }
BENCHMARK(BM_eCAL_Receive_Latency)->RangeMultiplier(RANGE_MULTIPLIER)->Range(RANGE_START, RANGE_LIMIT)->UseManualTime()->Unit(benchmark::kMicrosecond);
}


// Benchmark execution
BENCHMARK_MAIN();


/*
 * Options to reduce variance:
 * ->MinWarmUpTime()
 * ->Repetitions()
 * ->ComputeStatistics("min", [](const std::vector<double>& v) -> double {return *(std::min_element(std::begin(v), std::end(v)));})
 * 
*/