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

void OnReceive(const struct eCAL_STopicId* topic_id_, const struct eCAL_SDataTypeInformation* data_type_information_, const struct eCAL_SReceiveCallbackData* callback_data_, void* user_argument_) { 
  (void*)data_type_information_;
  const char* layer = (const char*)user_argument_;
  
  printf("---------------------------------------------------\n");
  printf(" Received %s message in C\n", layer);
  printf("---------------------------------------------------\n");
  printf(" Topic   : %s\n", topic_id_->topic_name);
  printf(" Size    : %lld\n",  callback_data_->buffer_size);
  printf(" Time    : %lld\n",  callback_data_->send_timestamp);
  printf(" Clock   : %lld\n",  callback_data_->send_clock);
  printf(" Message : %.*s\n", (int)(callback_data_->buffer_size), (char*)(callback_data_->buffer));
  printf("\n");
};

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

    Furthermore we set the default layer for future subscribers to SHM and deactivate the other layers.

    This will overwrite the settings that were set in the ecal.yaml file.
  */
  my_config->communication_mode = eCAL_eCommunicationMode_network;
  my_config->subscriber.layer.shm.enable = 1;
  my_config->subscriber.layer.udp.enable = 0;
  my_config->subscriber.layer.tcp.enable = 0;
  
  /*
    Now we can pass the configuration object to eCAL::Initialize().
  */
  eCAL_Initialize("cake receive c", NULL, my_config);

  /*
    Now we create subscribers that will communicat via different layers.
  */
  const char* topic_name = "cake";
  
  /*
    We start with the default configuration, which we set before via the configuration object.
    This means we will use the SHM layer for subscribing.
  */
  eCAL_Subscriber* subscriber_shm = eCAL_Subscriber_New(topic_name, NULL, NULL, NULL);

  eCAL_Subscriber_SetReceiveCallback(subscriber_shm, OnReceive, "SHM");
  
  /*
    Next we alter the configuration in order to have the communication via UDP.
  */
  struct eCAL_Subscriber_Configuration custom_subscriber_config;
  memcpy(&custom_subscriber_config, eCAL_GetSubscriberConfiguration(), sizeof(struct eCAL_Subscriber_Configuration));
  custom_subscriber_config.layer.udp.enable = 1;
  custom_subscriber_config.layer.shm.enable = 0;
 
  eCAL_Subscriber* subscriber_udp = eCAL_Subscriber_New(topic_name, NULL, NULL, &custom_subscriber_config);

  eCAL_Subscriber_SetReceiveCallback(subscriber_udp, OnReceive, "UDP");

  /*
    Last we create subscriber that will communicate via TCP.
  */
  custom_subscriber_config.layer.tcp.enable = 1;
  custom_subscriber_config.layer.udp.enable = 0;

  eCAL_Subscriber* subscriber_tcp = eCAL_Subscriber_New(topic_name, NULL, NULL, &custom_subscriber_config);

  eCAL_Subscriber_SetReceiveCallback(subscriber_tcp, OnReceive, "TCP");

  /*
    Stay in a loop and let the callbacks to their work until the user stops the program.
  */
  while(eCAL_Ok())
  {
    eCAL_Process_SleepMS(500);
  }

  /*
    New we delete all the handles we created.
  */
  eCAL_Subscriber_Delete(subscriber_shm);
  eCAL_Subscriber_Delete(subscriber_udp);
  eCAL_Subscriber_Delete(subscriber_tcp);
  eCAL_Configuration_Delete(my_config);

  /*
    And as always we need to finalize the eCAL API.
  */
  eCAL_Finalize();

  return(0);
}