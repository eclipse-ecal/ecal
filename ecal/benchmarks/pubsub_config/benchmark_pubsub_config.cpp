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

#include <thread>


#define REGISTRATION_DELAY_MS   2000
#define ACK_TIMEOUT_MS          50

#define RANGE_MULTIPLIER        1<<6
#define RANGE_START             1
#define RANGE_LIMIT             1<<24


/*
 *
 * Benchmarking the eCAL send process in zero-copy mode
 * 
*/
namespace Send_Zero_Copy {
  // Benchmark function
  static void BM_eCAL_Send_Zero_Copy(benchmark::State& state) {
    // Create payload to send, size depends on current argument
    size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    char* content_addr = content_vector.data();

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
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Zero_Copy)->RangeMultiplier(RANGE_MULTIPLIER)->Range(RANGE_START, RANGE_LIMIT)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process with handshake
 * 
*/
namespace Send_Handshake {
  // Benchmark function
  static void BM_eCAL_Send_Handshake(benchmark::State& state) {
    // Create payload to send, size depends on current argument
    size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.acknowledge_timeout_ms = ACK_TIMEOUT_MS;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Handshake)->RangeMultiplier(RANGE_MULTIPLIER)->Range(RANGE_START, RANGE_LIMIT)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process with double-buffering
 * 
*/
namespace Send_Double_Buffer {
  // Benchmark function
  static void BM_eCAL_Send_Double_Buffer(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    char* content_addr = content_vector.data();

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
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Double_Buffer)->RangeMultiplier(RANGE_MULTIPLIER)->Range(RANGE_START, RANGE_LIMIT)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process in zero-copy mode with handshake
 * 
*/
namespace Send_Zero_Copy_Handshake {
  // Benchmark function
  static void BM_eCAL_Send_Zero_Copy_Handshake(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.zero_copy_mode = true;
    pub_config.layer.shm.acknowledge_timeout_ms = ACK_TIMEOUT_MS;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Zero_Copy_Handshake)->RangeMultiplier(RANGE_MULTIPLIER)->Range(RANGE_START, RANGE_LIMIT)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process in zero-copy mode with double-buffering
 * 
*/
namespace Send_Zero_Copy_Double_Buffer {
  // Benchmark function
  static void BM_eCAL_Send_Zero_Copy_Double_Buffer(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    char* content_addr = content_vector.data();

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
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Zero_Copy_Double_Buffer)->RangeMultiplier(RANGE_MULTIPLIER)->Range(RANGE_START, RANGE_LIMIT)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process with double-buffering and handshake
 * 
*/
namespace Send_Double_Buffer_Handshake {
  // Benchmark function
  static void BM_eCAL_Send_Double_Buffer_Handshake(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.memfile_buffer_count = 2;
    pub_config.layer.shm.acknowledge_timeout_ms = ACK_TIMEOUT_MS;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Double_Buffer_Handshake)->RangeMultiplier(RANGE_MULTIPLIER)->Range(RANGE_START, RANGE_LIMIT)->UseRealTime();
}


/*
 *
 * Benchmarking the eCAL send process in zero-copy mode with double-buffering and handshake
 * 
*/
namespace Send_Double_Buffer_Handshake {
  // Benchmark function
  static void BM_eCAL_Send_Double_Buffer_Handshake(benchmark::State& state) {
    // Create payload to send, size depends on second argument
    size_t payload_size = state.range(0);
    std::vector<char> content_vector(payload_size);
    char* content_addr = content_vector.data();

    // Initialize eCAL
    eCAL::Initialize("Benchmark");

    // Create publisher config
    eCAL::Publisher::Configuration pub_config;
    pub_config.layer.shm.zero_copy_mode = true;
    pub_config.layer.shm.memfile_buffer_count = 2;
    pub_config.layer.shm.acknowledge_timeout_ms = ACK_TIMEOUT_MS;

    // Create publisher with config
    eCAL::CPublisher publisher("benchmark_topic", eCAL::SDataTypeInformation(), pub_config);

    // Create receiver in a different thread
    std::thread receiver_thread([]() { 
      eCAL::CSubscriber subscriber("benchmark_topic");
      while(eCAL::Ok()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    } );
    
    // Wait for eCAL synchronization
    std::this_thread::sleep_for(std::chrono::milliseconds(REGISTRATION_DELAY_MS));

    // This is the benchmarked section: Sending the payload
    for (auto _ : state) {
      publisher.Send(content_addr, payload_size);
    }

    // Finalize eCAL and wait for receiver thread to finish
    eCAL::Finalize();
    receiver_thread.join();
  }
  // Register the benchmark function
  BENCHMARK(BM_eCAL_Send_Double_Buffer_Handshake)->RangeMultiplier(RANGE_MULTIPLIER)->Range(RANGE_START, RANGE_LIMIT)->UseRealTime();
}