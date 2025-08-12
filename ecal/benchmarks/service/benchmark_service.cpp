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


constexpr int registration_delay_ms = 2000;


/*
 *
 * Benchmarking the eCAL call with response
 * 
*/
namespace Ping {
   // Define server service function
   int callback_ping(const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) {
      response_ = "response";
      return 0;
   }

   void BM_eCAL_Ping(benchmark::State& state) {
      // Initialize eCAL and create a server and a client
      eCAL::Initialize("Benchmark");
      eCAL::CServiceServer server("Server");
      const eCAL::CServiceClient client("Server", { {"ping", {}, {} } });

      // Set server service function
      server.SetMethodCallback({ "ping", {}, {} }, callback_ping);

      // Wait for connection
      std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));

      // Check if client instance exists
      if (client.GetClientInstances().size() == 0) { 
         std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));
         // Check again, exit if failed
         if (client.GetClientInstances().size() == 0) { std::exit(1); }
      }
 
      // This is the benchmarked section: Getting a response from the server
      for (auto _ : state) {
         client.GetClientInstances()[0].CallWithResponse("ping", "request", eCAL::CClientInstance::DEFAULT_TIME_ARGUMENT);
      }
      
      // Finalize eCAL
      eCAL::Finalize();
   }
   // Register the benchmark function
   BENCHMARK(BM_eCAL_Ping);
}

// Benchmark execution
BENCHMARK_MAIN();