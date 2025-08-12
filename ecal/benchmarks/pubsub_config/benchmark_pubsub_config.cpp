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
#include <benchmark/benchmark.h>

#include <random>
#include <thread>


constexpr int registration_delay_ms = 2000;
constexpr int ack_timeout_ms = 50;

constexpr int range_multiplier = 1 << 6;
constexpr int range_start = 1;
constexpr int range_limit = 1 << 24;


// Random byte generator
char gen() {
  static std::random_device rd;
  static std::mt19937 engine(rd());
  static std::uniform_int_distribution<> distr(0,255);
  return static_cast<char>(distr(engine));
}


/*
 *
 * Benchmarking the eCAL send process in zero-copy mode
 * 
*/
namespace Send_Zero_Copy {
  // Benchmark function
  void BM_eCAL_Send_Zero_Copy(benchmark::State& state) {
    // Create payload to send, size depends on current argument
    const size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    std::generate(content_vector.begin(), content_vector.end(), gen);
    const char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.zero_copy_mode = true;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Zero_Copy)->RangeMultiplier(range_multiplier)->Range(range_start, range_limit)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process with handshake
 * 
*/
namespace Send_Handshake {
  // Benchmark function
  void BM_eCAL_Send_Handshake(benchmark::State& state) {
    // Create payload to send, size depends on current argument
    const size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    std::generate(content_vector.begin(), content_vector.end(), gen);
    const char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.acknowledge_timeout_ms = ack_timeout_ms;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Handshake)->RangeMultiplier(range_multiplier)->Range(range_start, range_limit)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process with double-buffering
 * 
*/
namespace Send_Double_Buffer {
  // Benchmark function
  void BM_eCAL_Send_Double_Buffer(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    const size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    std::generate(content_vector.begin(), content_vector.end(), gen);
    const char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.memfile_buffer_count = 2;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Double_Buffer)->RangeMultiplier(range_multiplier)->Range(range_start, range_limit)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process in zero-copy mode with handshake
 * 
*/
namespace Send_Zero_Copy_Handshake {
  // Benchmark function
  void BM_eCAL_Send_Zero_Copy_Handshake(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    const size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    std::generate(content_vector.begin(), content_vector.end(), gen);
    const char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.zero_copy_mode = true;
    pub_config.layer.shm.acknowledge_timeout_ms = ack_timeout_ms;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Zero_Copy_Handshake)->RangeMultiplier(range_multiplier)->Range(range_start, range_limit)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process in zero-copy mode with double-buffering
 * 
*/
namespace Send_Zero_Copy_Double_Buffer {
  // Benchmark function
  void BM_eCAL_Send_Zero_Copy_Double_Buffer(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    const size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    std::generate(content_vector.begin(), content_vector.end(), gen);
    const char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.zero_copy_mode = true;
    pub_config.layer.shm.memfile_buffer_count = 2;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Zero_Copy_Double_Buffer)->RangeMultiplier(range_multiplier)->Range(range_start, range_limit)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process with double-buffering and handshake
 * 
*/
namespace Send_Double_Buffer_Handshake {
  // Benchmark function
  void BM_eCAL_Send_Double_Buffer_Handshake(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    const size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    std::generate(content_vector.begin(), content_vector.end(), gen);
    const char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.memfile_buffer_count = 2;
    pub_config.layer.shm.acknowledge_timeout_ms = ack_timeout_ms;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Double_Buffer_Handshake)->RangeMultiplier(range_multiplier)->Range(range_start, range_limit)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process in zero-copy mode with double-buffering and handshake
 * 
*/
namespace Send_Zero_Copy_Double_Buffer_Handshake {
  // Benchmark function
  void BM_eCAL_Send_Zero_Copy_Double_Buffer_Handshake(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    const size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    std::generate(content_vector.begin(), content_vector.end(), gen);
    const char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.zero_copy_mode = true;
    pub_config.layer.shm.memfile_buffer_count = 2;
    pub_config.layer.shm.acknowledge_timeout_ms = ack_timeout_ms;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Zero_Copy_Double_Buffer_Handshake)->RangeMultiplier(range_multiplier)->Range(range_start, range_limit)->UseRealTime();
}


// Benchmark execution
BENCHMARK_MAIN();