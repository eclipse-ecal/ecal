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
#include <ecal/msg/string/publisher.h>

#include <iostream>
#include <thread>
#include <vector>


class PublisherCreator
{
public:
  PublisherCreator(int publisher_count)
  {
    for (int i = 0; i < publisher_count; ++i)
    {
      publishers.emplace_back("Publisher" + std::to_string(i));
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
  std::vector<eCAL::string::CPublisher<std::string>> publishers;

};


int main(int argc, char** argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "many_connections_send");
  PublisherCreator publishers(1000);
  std::cout << "Done Initializing" << std::endl;

  while (eCAL::Ok())
  {
    publishers.SendAll();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  eCAL::Finalize();

}