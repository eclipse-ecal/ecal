/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include <chrono>
#include <iostream>
#include <string>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable protobuf warnings
#endif
#include <ecal/core/pb/monitoring.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define MEASURE_VARIANT_STRING 1
#define MEASURE_VARIANT_STRUCT 1

int main(int argc, char **argv)
{
  // initialize eCAL core API
  eCAL::Initialize(argc, argv, "monitoring performance", eCAL::Init::All);

  int                                   runs(100);
  int                                   run(0);
  std::chrono::steady_clock::time_point start_time;

  // monitor for ever
  while(eCAL::Ok())
  {
#if MEASURE_VARIANT_STRING
    // take snapshots as string (and parse it afterwards to protobuf)
    {
      int num_topics(0);
      start_time = std::chrono::steady_clock::now();
      for (run = 0; run < runs; ++run)
      {
        std::string          monitoring_s;
        eCAL::pb::Monitoring monitoring;
        eCAL::Monitoring::GetMonitoring(monitoring_s);
        monitoring.ParseFromString(monitoring_s);
        num_topics = monitoring.topics_size();
      }

      auto diff_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
      std::cout << "Monitoring time to string  : " << static_cast<double>(diff_time.count()) / runs << " ms"  << " (" << num_topics << " topics)" << std::endl;
    }
#endif // MEASURE_VARIANT_STRING

#if MEASURE_VARIANT_STRUCT
    // take snapshots as monitoring struct
    {
      size_t num_topics(0);
      start_time = std::chrono::steady_clock::now();
      for (run = 0; run < runs; ++run)
      {
        eCAL::Monitoring::SMonitoring monitoring;
        eCAL::Monitoring::GetMonitoring(monitoring);
        num_topics = monitoring.publisher.size() + monitoring.subscriber.size();
      }
      auto diff_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
      std::cout << "Monitoring time to structs : " << static_cast<double>(diff_time.count()) / runs << " ms" << " (" << num_topics << " topics)" << std::endl;
    }
#endif // MEASURE_VARIANT_STRUCT

    std::cout << std::endl;
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
