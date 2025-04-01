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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
  printf("-------------------------------\n");
  printf(" C: HELLO WORLD SENDER\n");
  printf("-------------------------------\n");

  /*
    We create the objects we want to work with.
    In this case we need a publisher handle and a data type information structure.
    Additionally we need a string buffer to hold the content we want to send.
  */
  eCAL_Publisher* publisher;
  struct eCAL_SDataTypeInformation data_type_information;
  char snd_s[256];
  int cnt = 0;

  /*
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "hello_send_c". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL_Initialize("hello_send_c", NULL, NULL);

  /*
    We set all data in eCAL_SDataTypeInformation to zero.
    We want to publish raw strings, so we set the name of the data type information
    to "string" and the encoding to "utf-8".
  */
  memset(&data_type_information, 0, sizeof(struct eCAL_SDataTypeInformation));
  data_type_information.name     = "string";
  data_type_information.encoding = "utf-8";

  /*
    Now we create a new publisher that will publish the topic "hello".
    Furthermore we set the data type information of the publisher.
    The two additional parameters that could be set (eCAL_PubEventCallbackT and eCAL_Publisher_Configuration)
    are set to NULL, because for this example we are fine with the default settings.
  */
  publisher = eCAL_Publisher_New("hello", &data_type_information, NULL, NULL);

  /*
    Get the topic id of the publisher by using eCAL_Publisher_GetTopicId and print it.
  */
  printf("Publisher id: %ul\n\n", (unsigned long)eCAL_Publisher_GetTopicId(publisher)->topic_id.entity_id);
  
  /*
    Creating an infinite publish-loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL_Ok() will return false.
  */
  while(eCAL_Ok())
  {
    /*
      Build the string you want to send, using snprintf in this example.
      The string will be "HELLO WORLD FROM C (1)", "HELLO WORLD FROM C (2)", ...
    */
    snprintf(snd_s, sizeof(snd_s), "HELLO WORLD FROM C (%d)", ++cnt);

    /*
      Send the content to other eCAL Processes that have subscribed to the topic "hello".
      The message is sent as a raw string, so we use the length of the string as the size of the message.
    */
    if(!eCAL_Publisher_Send(publisher, snd_s, strlen(snd_s), NULL))
      printf("Published topic \"Hello\" with \"%s\"\n", snd_s);
    else
      printf("Sending topic \"Hello\" failed !\n");

    /*
      Sleep for 500 ms so we send with a frequency of 2 Hz.
    */
    eCAL_Process_SleepMS(500);
  }

  /*
    Now we delete the publisher handle. This is important to free resources.
    All objects that are created with eCAL_xxx_New() have to be deleted with eCAL_xxx_Delete().
  */
  eCAL_Publisher_Delete(publisher);

  
  /*
    Deinitialize eCAL.
    You should always do that before your application exits.
  */
  eCAL_Finalize();

  return(0);
}