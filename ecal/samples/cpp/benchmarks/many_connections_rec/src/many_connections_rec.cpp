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

class SubscriberCreator
{
public:
  SubscriberCreator(int publisher_count)
  {
    const std::string ttype("THIS IS THE TOPIC TYPE NAME");
    std::string       tdesc("THIS IS THE LONG TOPIC DESCRIPTOR ");
    for (auto rep = 0; rep < 4; ++rep) tdesc = tdesc.append(tdesc);

    for (int i = 0; i < publisher_count; ++i)
    {
      std::ostringstream topic_name;
      topic_name << std::setw(5) << std::setfill('0') << i;
      subscribers.emplace_back("Topic" + topic_name.str(), eCAL::SDataTypeInformation{ ttype, "", tdesc });
      auto on_receive = [this](const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData&)
      {
        Receive();
      };
      subscribers.at(i).SetReceiveCallback(on_receive);
    }
  }

  void Receive()
  {
  }

private:
  std::vector<eCAL::CSubscriber> subscribers;
};


int main()
{
  // initialize eCAL API
  eCAL::Initialize("many_connections_rec");

  // create many subscriber
  const SubscriberCreator subscribers(10000);
  std::cout << "Done Initializing" << std::endl;

  while (eCAL::Ok())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
