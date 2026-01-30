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

// Include the basic eCAL header
#include <ecal_c/ecal.h>

#include <string.h> //memset()
#include <stdio.h>  //printf()

int main()
{
  /*
    Now we create a configuration object.
    By default, when you use the eCAL::Init::Configuration() function, the ecal.yaml file will be read.
    With that you have your main systems configuration.

    If you want to use the defaults only, you can use directly eCAL::Configuration struct and use it the same way.
  */
  eCAL_Configuration* my_config = eCAL_Init_Configuration();

  /*
    Let's set some configuration parameters.
    We want our eCAL application to communicate over the network (e.g. to monitor it from another machine).

    Furthermore we set the default layer for future publishers and subscribers to SHM and deactivate the other layers.

    This will overwrite the settings that were set in the ecal.yaml file.
  */
  my_config->communication_mode = eCAL_eCommunicationMode_network;
  my_config->publisher.layer.shm.enable = 1;
  my_config->publisher.layer.udp.enable = 0;
  my_config->publisher.layer.tcp.enable = 0;
  
  /*
    Now we can pass the configuration object to eCAL::Initialize().
  */
  eCAL_Initialize("cake send c", NULL, my_config);

  /*
    Now we create publishers and subscribers that will communicat via different layers.
  */
  const char* topic_name = "cake";
  
  /*
    We start with the default configuration, which we set before via the configuration object.
    This means we will use the SHM layer for publishing and subscribing.
  */
  eCAL_Publisher* publisher_shm = eCAL_Publisher_New(topic_name, NULL, NULL, NULL);
  
  /*
    Next we alter the configuration in order to have the communication via UDP.
  */
  struct eCAL_Publisher_Configuration custom_publisher_config;
  memcpy(&custom_publisher_config, eCAL_GetPublisherConfiguration(), sizeof(struct eCAL_Publisher_Configuration));
  custom_publisher_config.layer.udp.enable = 1;
  custom_publisher_config.layer.shm.enable = 0;
 
  eCAL_Publisher* publisher_udp = eCAL_Publisher_New(topic_name, NULL, NULL, &custom_publisher_config);

  /*
    Last we create a publisher and subscriber that will communicate via TCP.
  */
  custom_publisher_config.layer.tcp.enable = 1;
  custom_publisher_config.layer.udp.enable = 0;

  eCAL_Publisher* publisher_tcp = eCAL_Publisher_New(topic_name, NULL, NULL, &custom_publisher_config);

  /*
    Now we can start the publisher and subscriber.
    The publisher will send a message every 500ms.
    The subscriber will print the received messages to the console.
  */
  unsigned int counter = 0;
  char message_shm[256];
  char message_udp[256];
  char message_tcp[256];

  while(eCAL_Ok())
  {
    /*
      Now we send the messages via the different layers.
      Only the subscriber that listens to the same layer will receive the message.

      We will have some delay between the messages, so there will be overlapping messages.
    */
    snprintf(message_shm, sizeof(message_shm), "Hello from SHM publisher (%d)", counter);
    eCAL_Publisher_Send(publisher_shm, message_shm, strlen(message_shm), NULL);
    eCAL_Process_SleepMS(100);

    snprintf(message_udp, sizeof(message_udp), "Hello from UDP publisher (%d)", counter);
    eCAL_Publisher_Send(publisher_udp, message_udp, strlen(message_udp), NULL);
    eCAL_Process_SleepMS(100);

    snprintf(message_tcp, sizeof(message_tcp), "Hello from TCP publisher (%d)", counter);
    eCAL_Publisher_Send(publisher_tcp, message_tcp, strlen(message_tcp), NULL);

    ++counter;

    eCAL_Process_SleepMS(500);
  }

  /*
    New we delete all the handles we created.
  */
  eCAL_Publisher_Delete(publisher_shm);
  eCAL_Publisher_Delete(publisher_udp);
  eCAL_Publisher_Delete(publisher_tcp);
  eCAL_Configuration_Delete(my_config);

  /*
    And as always we need to finalize the eCAL API.
  */
  eCAL_Finalize();

  return(0);
}