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

#include <stdio.h> //printf()
#include <string.h> //memcpy(), memset()


int OnMethodCallback(const struct eCAL_SServiceMethodInformation* method_info_, const void* request_, size_t request_length_, void** response_, size_t* response_length_, void* user_argument_)
{
  (void)user_argument_;

  // In order pass the server response properly to the callback API, the underlying memory needs to be allocated 
  // with eCAL_Malloc(). The allocation via eCAL_Malloc() is required as the internal memory handler frees the
  // resevered memory after callback execution.
  *response_ = eCAL_Malloc(request_length_);

  // In case of a failure, the value that response_ points to, remains NULL.
  if (*response_ == NULL) return 1; // memory allocation failed

  // The length of response buffer needs to be set accordingly
  *response_length_ = request_length_;

  // In this example the entire request buffer will be copied over to the response buffer.
  memcpy(*response_, request_, request_length_);


  printf("Method   : %s called\n", method_info_->method_name);
  printf("Request  : %s\n",        (char*)request_);
  printf("Response : %s\n",        (char*)(*response_));
  printf("\n");

  // Zero can be returned here as the callback has been successfully proceeded.
  return 0;
}

int main()
{
  eCAL_ServiceServer *server;
  struct eCAL_SServiceMethodInformation method_information;

  // Iinitialize eCAL API
  eCAL_Initialize("minimal server c", NULL, NULL);

  // Create server "service1"
  server = eCAL_ServiceServer_New("service1", NULL);

  // Define all available service methods by assiging the required fields of ServiceMethodInformation (e.g. method name) and attach the respective callback functions
  memset(&method_information, 0, sizeof(struct eCAL_SServiceMethodInformation));
  method_information.method_name = "echo";
  eCAL_ServiceServer_SetMethodCallback(server, &method_information, OnMethodCallback, NULL);

  // Idle
  while (eCAL_Ok())
  {
    // Sleep for 100 ms
    eCAL_Process_SleepMS(100);
  }

  // Destroy server "service1"
  eCAL_ServiceServer_Delete(server);

  // Finalize eCAL API
  eCAL_Finalize();

  return 0;
}
