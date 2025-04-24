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
  char message[256];
  int loop_count = 0;

  /*
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "hello send c". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL_Initialize("hello send c", NULL, NULL);

  /*
    Print some eCAL version information.
  */
  printf("eCAL %s (%s)", eCAL_GetVersionString(), eCAL_GetVersionDateString());

  /*
    Set the state for the program.
    You can vary between different states like healthy, warning, critical ...
    This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  */
  eCAL_Process_SetState(eCAL_Process_eSeverity_healthy, eCAL_Process_eSeverityLevel_level1, "I feel good!");

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
    Creating an infinite publish-loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL_Ok() will return false.
  */
  while(eCAL_Ok())
  {
    /*
      Build the string you want to send, using snprintf in this example.
      The string will be "HELLO WORLD FROM C (1)", "HELLO WORLD FROM C (2)", ...
    */
    snprintf(message, sizeof(message), "HELLO WORLD FROM C (%d)", ++loop_count);

    /*
      Send the content to other eCAL Processes that have subscribed to the topic "hello".
      eCAL transports messages as a byte array and length, so for interop with other languages, 
      we will send a non-null terminated strings over the wire.
    */
    if(!eCAL_Publisher_Send(publisher, message, strlen(message), NULL))
      printf("Sent string message in C: \"%s\"\n", message);
    else
      printf("Sending string message in C failed!\n");

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