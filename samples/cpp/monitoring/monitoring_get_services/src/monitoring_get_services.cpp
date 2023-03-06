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
#include <map>
#include <vector>

int main(int argc, char **argv)
{
  int                                   run(0), runs(100000);
  std::chrono::steady_clock::time_point start_time;

  // initialize eCAL core API
  eCAL::Initialize(argc, argv, "monitoring get services");

  // monitor for ever
  while(eCAL::Ok())
  {
    // GetServices
    {
      std::map<std::tuple<std::string, std::string>, eCAL::Util::SServiceMethodInfo> service_info_map;

      start_time = std::chrono::steady_clock::now();
      for (run = 0; run < runs; ++run)
      {
        eCAL::Util::GetServices(service_info_map);
      }

      auto num_services = service_info_map.size();
      auto diff_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
      std::cout << "GetServices      : " << static_cast<double>(diff_time.count()) / runs << " ms" << " (" << num_services << " services)" << std::endl;
      std::cout << std::endl;
    }

    // GetServiceNames
    {
      std::vector<std::tuple<std::string, std::string>> service_method_names;

      start_time = std::chrono::steady_clock::now();
      for (run = 0; run < runs; ++run)
      {
        eCAL::Util::GetServiceNames(service_method_names);
      }

      auto num_services = service_method_names.size();
      auto diff_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
      std::cout << "GetServicesNames : " << static_cast<double>(diff_time.count()) / runs << " ms" << " (" << num_services << " services)" << std::endl;
      std::cout << std::endl;
    }
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
