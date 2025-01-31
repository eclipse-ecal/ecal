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

#include <algorithm>
#include <iostream>
#include <chrono>
#include <numeric>
#include <thread>
#include <vector>

// main entry
int main()
{
  // initialize eCAL API
  eCAL::Initialize("latency client");

  // create latency client
  eCAL::CServiceClient latency_client("latency");

  // waiting for service
  while (eCAL::Ok() && !latency_client.IsConnected())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Waiting for the service .." << std::endl;
  }
  std::cout << std::endl << "Start measurement" << std::endl << std::endl;

  // prepare latency array
  const int calls(1000);
  const int subcalls(100);
  std::vector<long long> latency_array;
  latency_array.reserve(calls);

  // run it
  int run(calls);
  while (eCAL::Ok() && run--)
  {
    // take start time
    long long start_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    // call service method "hello" subcalls times (for better time accuracy)
    for (auto i = 0; i < subcalls; ++i)
    {
      latency_client.CallWithCallback("hello", "", eCAL::ResponseCallbackT());
    }
    
    // take return time and store it into the latency array
    long long return_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    latency_array.emplace_back((return_time - start_time)/subcalls);
  }

  // calculate sum and average time
  long long sum_time = std::accumulate(latency_array.begin(), latency_array.end(), 0LL);
  long long avg_time = sum_time / latency_array.size();

  // calculate min and max time
  auto      min_it = std::min_element(latency_array.begin(), latency_array.end());
  auto      max_it = std::max_element(latency_array.begin(), latency_array.end());
  size_t    min_pos = min_it - latency_array.begin();
  size_t    max_pos = max_it - latency_array.begin();
  long long min_time = *min_it;
  long long max_time = *max_it;

  // log result
  std::cout << "Service calls                : " << calls*subcalls                  << std::endl;
  std::cout << "Service call average latency : " << avg_time << " us"               << std::endl;
  std::cout << "Service call min latency     : " << min_time << " us @ " << min_pos << std::endl;
  std::cout << "Service call max latency     : " << max_time << " us @ " << max_pos << std::endl;
  std::cout << "Service throughput           : " << static_cast<int>((calls * subcalls) / (sum_time / 1000.0 / 1000.0)) << " calls/s" << std::endl;

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
