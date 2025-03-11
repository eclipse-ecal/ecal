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
#include <stdlib.h> //malloc

int OnMethodCallback(const struct eCAL_SServiceMethodInformation* method_info_, const void* request_, size_t request_length_, void** response_, size_t* response_length_)
{
  *response_ = malloc(request_length_);
  if (*response_ == NULL) return 1; // memory allocation failed

  // echo request to response
  memcpy(*response_, request_, request_length_);
  *response_length_ = request_length_;

  printf("Method   : %s called\n", method_info_->method_name);
  printf("Request  : %s\n",        (char*)request_);
  printf("Response : %s\n",        (char*)(*response_));
  printf("\n");

  // return success
  return 0;
}

int main()
{
  eCAL_ServiceServer *server;
  struct eCAL_SServiceMethodInformation method_information;

  // initialize eCAL API
  eCAL_Initialize("minimal server c", NULL);

  // create server "service1"
  server = eCAL_ServiceServer_New("service1", NULL);

  // add method callback for method "echo"

  memset(&method_information, 0, sizeof(struct eCAL_SServiceMethodInformation));
  method_information.method_name = "echo";
  eCAL_ServiceServer_SetMethodCallback(server, &method_information, OnMethodCallback);

  // idle
  while (eCAL_Ok())
  {
    // sleep 100 ms
    eCAL_Process_SleepMS(100);
  }

  // destroy server "service1"
  eCAL_ServiceClient_Delete(server);

  // finalize eCAL API
  eCAL_Finalize();

  return(0);
}
