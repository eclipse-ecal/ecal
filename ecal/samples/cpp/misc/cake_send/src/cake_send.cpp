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
  In addition, as we want to send simple strings, we include the string publisher header.
*/
#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>

#include <iostream>

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

    Furthermore we set the default layer for future publishers to SHM and deactivate the other layers.

    This will overwrite the settings that were set in the ecal.yaml file.
  */
  my_config.communication_mode = eCAL::eCommunicationMode::network;
  my_config.publisher.layer.shm.enable = true;
  my_config.publisher.layer.udp.enable = false;
  my_config.publisher.layer.tcp.enable = false;
  
  /*
    Now we can pass the configuration object to eCAL::Initialize().
  */
  eCAL::Initialize(my_config, "cake send");

  /*
    Now we create publishers that will communicat via different layers.
  */
  const std::string topic_name = "cake";
  
  /*
    We start with the default configuration, which we set before via the configuration object.
    This means we will use the SHM layer for publishing.
  */
  eCAL::string::CPublisher publisher_shm(topic_name);

  /*
    Next we alter the configuration in order to have the communication via UDP.
  */
  auto custom_publisher_config = eCAL::GetPublisherConfiguration();
  custom_publisher_config.layer.udp.enable = true;
  custom_publisher_config.layer.shm.enable = false;
 
  eCAL::string::CPublisher publisher_udp(topic_name, custom_publisher_config);

  /*
    Last we create a publisher that will communicate via TCP.
  */
  custom_publisher_config.layer.tcp.enable = true;
  custom_publisher_config.layer.udp.enable = false;

  eCAL::string::CPublisher publisher_tcp(topic_name, custom_publisher_config);

  /*
    Now we can start the publisher.
    The publisher will send a message every 500ms.
  */
  unsigned int counter = 0;

  while(eCAL::Ok())
  {
    /*
      Now we send the messages via the different layers.
      Only the subscriber that listens to the same layer will receive the message.

      We will have some delay between the messages, so there will be overlapping messages.
    */
    publisher_shm.Send("Hello from SHM publisher " + std::to_string(counter));
    eCAL::Process::SleepMS(100);
    publisher_udp.Send("Hello from UDP publisher " + std::to_string(counter));
    eCAL::Process::SleepMS(100);
    publisher_tcp.Send("Hello from TCP publisher " + std::to_string(counter));

    ++counter;

    eCAL::Process::SleepMS(300);
  }

  /*
    And as always we need to finalize the eCAL API.
  */
  eCAL::Finalize();

  return(0);
}