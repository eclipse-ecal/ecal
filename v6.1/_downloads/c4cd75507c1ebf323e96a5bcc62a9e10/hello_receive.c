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


/*
  Here we create the receive callback function.
  The function will be called whenever a new message is received.
*/
void OnReceive(const struct eCAL_STopicId* topic_id_, const struct eCAL_SDataTypeInformation* data_type_information_, const struct eCAL_SReceiveCallbackData* callback_data_, void* user_argument_)
{
  /*
    These are unused arguments for this example.
    You can use the data type information to check the data type of the received message.
    The user argument can be used to work with user defined data when the callback is called.
  */
  (void)data_type_information_;
  (void)user_argument_;

  printf("---------------------------------------------------\n");
  printf(" Received string message from topic \"%s\" in C\n", topic_id_->topic_name);
  printf("---------------------------------------------------\n");
  printf(" Size    : %zu\n",  callback_data_->buffer_size);
  printf(" Time    : %lld\n",  callback_data_->send_timestamp);
  printf(" Clock   : %lld\n",  callback_data_->send_clock);
  printf(" Message : %.*s\n", (int)(callback_data_->buffer_size), (char*)(callback_data_->buffer));
  printf("\n");
}

int main()
{
  printf("-------------------------------\n");
  printf(" C: HELLO WORLD RECEIVER\n");
  printf("-------------------------------\n");

  /*
    We create the objects we want to work with.
    In this case we need a subscriber handle and a data type information structure.
  */
  eCAL_Subscriber *subscriber;
  struct eCAL_SDataTypeInformation data_type_information;

  /*
    Initialize eCAL. You always have to initialize eCAL before using its API.
    The name of our eCAL Process will be "hello receive c". 
    This name will be visible in the eCAL Monitor, once the process is running.
  */
  eCAL_Initialize("hello receive c", NULL, NULL);

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
    We want to receive raw strings, so we set the name of the data type information
    to "string" and the encoding to "utf-8".
  */
  memset(&data_type_information, 0, sizeof(struct eCAL_SDataTypeInformation));
  data_type_information.name     = "string";
  data_type_information.encoding = "utf-8";
  
  /*
    Now we create a new subscriber that will subscribe to the topic "hello".
    Furthermore we set the data type information of the subscriber.
    The two additional parameters that could be set (eCAL_SubEventCallbackT and eCAL_Subscriber_Configuration)
    are set to NULL, because for this example we are fine with the default settings.
  */
  subscriber = eCAL_Subscriber_New("hello", &data_type_information, NULL, NULL);

  /*
    In order to receive message, we need to register a receive callback.
    The callback will be called whenever a new message is received.
  */
  eCAL_Subscriber_SetReceiveCallback(subscriber, OnReceive, NULL);

  /*
    Creating an infinite loop.
    eCAL Supports a stop signal; when an eCAL Process is stopped, eCAL_Ok() will return false.
  */
  while(eCAL_Ok())
  {
    /*
      Sleep for 500ms to avoid busy waiting.
      You can use eCAL_Process_SleepMS() to sleep in milliseconds.
    */
    eCAL_Process_SleepMS(500);
  }

  /*
    Now we delete the subscriber handle. This is important to free resources.
    All objects that are created with eCAL_xxx_New() have to be deleted with eCAL_xxx_Delete().
  */
  eCAL_Subscriber_Delete(subscriber);

  /*
    Deinitialize eCAL.
    You should always do that before your application exits.
  */
  eCAL_Finalize();

  return(0);
}
