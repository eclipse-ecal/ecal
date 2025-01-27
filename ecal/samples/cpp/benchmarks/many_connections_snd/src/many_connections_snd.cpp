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

#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

class PublisherCreator
{
public:
  PublisherCreator(int publisher_count)
  {
    const std::string ttype("THIS IS THE TOPIC TYPE NAME");
    std::string       tdesc("THIS IS THE LONG TOPIC DESCRIPTOR ");
    for (auto rep = 0; rep < 4; ++rep) tdesc = tdesc.append(tdesc);

    for (int i = 1; i <= publisher_count; ++i)
    {
      std::ostringstream topic_name;
      topic_name << std::setw(5) << std::setfill('0') << i;
      publishers.emplace_back("Topic" + topic_name.str(), eCAL::SDataTypeInformation{ ttype, "",  tdesc });
    }
  }

  void SendAll()
  {
    for (auto& pub : publishers)
    {
      pub.Send("Hello World");
    }
  }

private:
  std::vector<eCAL::CPublisher> publishers;
};


int main()
{
  // initialize eCAL API
  eCAL::Initialize("many_connections_snd");

  // create many publisher
  PublisherCreator publishers(10000);
  std::cout << "Done Initializing" << std::endl;

  while (eCAL::Ok())
  {
    publishers.SendAll();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
