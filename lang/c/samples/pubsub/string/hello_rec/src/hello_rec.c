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

#include <ecal_c/ecal.h>

#include <string.h> //memset()
#include <stdio.h> //printf()

void OnReceive(const struct eCAL_STopicId* topic_id_, const struct eCAL_SDataTypeInformation* data_type_information_, const struct eCAL_SReceiveCallbackData* callback_data_, void* user_argument_)
{
  // unused arguments
  (void)data_type_information_;
  (void)user_argument_;

  printf("Received topic \"%s\" with ", topic_id_->topic_name);
  printf("\"%.*s\"\n", (int)(callback_data_->buffer_size), (char*)(callback_data_->buffer));
}

int main()
{
  eCAL_Subscriber *subscriber;
  struct eCAL_SDataTypeInformation data_type_information;

  // initialize eCAL API
  eCAL_Initialize("hello_snd_c", NULL, NULL);

  // create subscriber "Hello"
  memset(&data_type_information, 0, sizeof(struct eCAL_SDataTypeInformation));
  data_type_information.name = "string";
  data_type_information.encoding = "utf-8";
  
  subscriber = eCAL_Subscriber_New("hello", &data_type_information, NULL, NULL);

  // add callback
  eCAL_Subscriber_SetReceiveCallback(subscriber, OnReceive, NULL);

  printf("Subscriber id: %ul\n\n", (unsigned long)eCAL_Subscriber_GetTopicId(subscriber)->topic_id.entity_id);

  // idle main thread
  while(eCAL_Ok())
  {
    // sleep 100 ms
    eCAL_Process_SleepMS(100);
  }

  // destroy subscriber
  eCAL_Subscriber_Delete(subscriber);

  // finalize eCAL API
  eCAL_Finalize();

  return(0);
}
