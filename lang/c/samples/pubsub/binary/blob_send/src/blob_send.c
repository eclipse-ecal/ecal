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
#include <time.h>

#define BUFFER_SIZE 16

/*
  Some helper function to generate binary data into a binary_buffer.
  Clears the vector, resizes it to a specified size and fills it with random ascii characters.
*/
void fillBinaryBuffer(unsigned char* binary_buffer, size_t buffer_size) 
{
  unsigned int milliseconds = (unsigned int)(clock());
  srand(milliseconds); // Seed with milliseconds

   for (size_t i = 0; i < buffer_size; ++i) 
   {
    binary_buffer[i] = (unsigned char)(rand() % 95 + 32); // ASCII range 32-126
   }

}

int main()
{
  printf("----------------\n");
  printf(" C: BLOB SENDER\n");
  printf("----------------\n");

  /*
    We create the objects we want to work with.
    In this case we need a publisher handle.
    Additionally we need a char binary_buffer to hold the content we want to send.
  */
  eCAL_Publisher* publisher;
  unsigned char binary_buffer[BUFFER_SIZE];

  /*
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "blob send c". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL_Initialize("blob send c", NULL, NULL);

  /*
    Print some eCAL version information.
  */
  printf("eCAL %s (%s)\n", eCAL_GetVersionString(), eCAL_GetVersionDateString());

  /*
    Set the state for the program.
    You can vary between different states like healthy, warning, critical ...
    This can be used to communicate the application state to applications like eCAL Monitor/Sys.
  */
  eCAL_Process_SetState(eCAL_Process_eSeverity_healthy, eCAL_Process_eSeverityLevel_level1, "I feel good!");

  /*
    Now we create a new publisher that will publish the topic "blob".
    Furthermore we set the data type information of the publisher.
    The two additional parameters that could be set (eCAL_PubEventCallbackT and eCAL_Publisher_Configuration)
    are set to NULL, because for this example we are fine with the default settings.
  */
  publisher = eCAL_Publisher_New("blob", NULL, NULL, NULL);
  
  /*
    Creating an infinite publish-loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL_Ok() will return false.
  */
  while(eCAL_Ok())
  {
    /*
      Build the blob you want to send. You can add any binary data here, we fill the message with the loop count.
    */
    fillBinaryBuffer(binary_buffer, BUFFER_SIZE);

    /*
      Send the content to other eCAL Processes that have subscribed to the topic "blob".
      The message is sent as binary, so we use the size of the message as the size of the message to send.
    */
    if(!eCAL_Publisher_Send(publisher, binary_buffer, BUFFER_SIZE, NULL))
      printf("Sent binary data in C: \"%.*s\"\n", BUFFER_SIZE, binary_buffer);
    else
      printf("Sending binary data in C failed!\n");

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