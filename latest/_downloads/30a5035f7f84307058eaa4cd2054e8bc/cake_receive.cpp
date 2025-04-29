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

/*
  Include the eCAL main header.
  In addition, as we want to send simple strings, we include the string subscriber header.
*/
#include <ecal/ecal.h>
#include <ecal/msg/string/subscriber.h>

#include <iostream>

void printLayerHeader(const std::string& layer_name_) {
  std::cout << "---------------------------------------------------" << "\n";
  std::cout << " Received " << layer_name_ << " message in C++" << "\n";
  std::cout << "---------------------------------------------------" << "\n";
};

void printCallbackInformation(const eCAL::STopicId& publisher_id_, const std::string& message_, long long time_, long long clock_) { 
  std::cout << " Topic   : " << publisher_id_.topic_name << "\n";
  std::cout << " Size    : " << message_.size()          << "\n";
  std::cout << " Time    : " << time_                    << "\n";
  std::cout << " Clock   : " << clock_                   << "\n";
  std::cout << " Message : " << message_                 << "\n";
  std::cout << "\n";
};

int main()
{
  /*
    Now we create a configuration object.
    By default, when you use the eCAL::Init::Configuration() function, the ecal.yaml file will be read.
    With that you have your main systems configuration.

    If you want to use the defaults only, you can use directly eCAL::Configuration struct and use it the same way.
  */
  eCAL::Configuration my_config = eCAL::Init::Configuration();

  /*
    Let's set some configuration parameters.
    We want our eCAL application to communicate over the network (e.g. to monitor it from another machine).

    Furthermore we set the default layer for future subscribers to SHM and deactivate the other layers.

    This will overwrite the settings that were set in the ecal.yaml file.
  */
  my_config.communication_mode = eCAL::eCommunicationMode::network;
  my_config.subscriber.layer.shm.enable = true;
  my_config.subscriber.layer.udp.enable = false;
  my_config.subscriber.layer.tcp.enable = false;
  
  /*
    Now we can pass the configuration object to eCAL::Initialize().
  */
  eCAL::Initialize(my_config, "cake receive");

  /*
    Now we create subscribers that will communicat via different layers.
  */
  const std::string topic_name = "cake";
  
  /*
    We start with the default configuration, which we set before via the configuration object.
    This means we will use the SHM layer for subscribing.
  */
  eCAL::string::CSubscriber subscriber_shm(topic_name);
  subscriber_shm.SetReceiveCallback([](const eCAL::STopicId& publisher_id_, const std::string& message_, long long time_, long long clock_) { 
    printLayerHeader("SHM");
    printCallbackInformation(publisher_id_, message_, time_, clock_);
  });

  /*
    Next we alter the configuration in order to have the communication via UDP.
  */
  auto custom_subscriber_config = eCAL::GetSubscriberConfiguration();
  custom_subscriber_config.layer.udp.enable = true;
  custom_subscriber_config.layer.shm.enable = false;
 
  eCAL::string::CSubscriber subscriber_udp(topic_name, custom_subscriber_config);

  subscriber_udp.SetReceiveCallback([](const eCAL::STopicId& publisher_id_, const std::string& message_, long long time_, long long clock_) { 
    printLayerHeader("UDP");
    printCallbackInformation(publisher_id_, message_, time_, clock_);
  });

  /*
    Last we create a subscriber that will communicate via TCP.
  */
  custom_subscriber_config.layer.tcp.enable = true;
  custom_subscriber_config.layer.udp.enable = false;

  eCAL::string::CSubscriber subscriber_tcp(topic_name, custom_subscriber_config);

  subscriber_tcp.SetReceiveCallback([](const eCAL::STopicId& publisher_id_, const std::string& message_, long long time_, long long clock_) { 
    printLayerHeader("TCP");
    printCallbackInformation(publisher_id_, message_, time_, clock_);
  });

  /*
    Stay in a loop and let the callbacks to their work until the user stops the program.
  */
  while(eCAL::Ok())
  {
    eCAL::Process::SleepMS(500);
  }

  /*
    And as always we need to finalize the eCAL API.
  */
  eCAL::Finalize();

  return(0);
}