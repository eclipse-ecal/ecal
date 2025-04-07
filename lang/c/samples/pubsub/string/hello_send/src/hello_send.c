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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
  eCAL_Publisher* publisher;
  struct eCAL_SDataTypeInformation data_type_information;
  char snd_s[256];
  int cnt = 0;

  // initialize eCAL API
  eCAL_Initialize("hello_send_c", NULL, NULL);

  // create publisher "Hello"
  memset(&data_type_information, 0, sizeof(struct eCAL_SDataTypeInformation));
  data_type_information.name = "string";
  data_type_information.encoding = "utf-8";

  publisher = eCAL_Publisher_New("hello", &data_type_information, NULL, NULL);

  printf("Publisher id: %ul\n\n", (unsigned long)eCAL_Publisher_GetTopicId(publisher)->topic_id.entity_id);
  
  // send updates
  while(eCAL_Ok())
  {
    // create message
    snprintf(snd_s, sizeof(snd_s), "HELLO WORLD FROM C (%d)", ++cnt);

    // send content
    if(!eCAL_Publisher_Send(publisher, snd_s, strlen(snd_s), NULL))
      printf("Published topic \"Hello\" with \"%s\"\n", snd_s);
    else
      printf("Sending topic \"Hello\" failed !\n");

    // sleep 500 ms
    eCAL_Process_SleepMS(500);
  }

  // delete publisher handle
  eCAL_Publisher_Delete(publisher);

  // finalize eCAL API
  eCAL_Finalize();

  return(0);
}