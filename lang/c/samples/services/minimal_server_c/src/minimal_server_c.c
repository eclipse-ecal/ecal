/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

int OnMethodCallback(const char* method_, const char* req_type_, const char* resp_type_, const char* request_, int request_len_, void** response_, int* response_len_, void* par_)
{
  // unused param
  (void)req_type_;
  (void)resp_type_;
  (void)par_;

  static char response_buf[1024];
  if ((unsigned int)request_len_ > sizeof(response_buf)) return 0;

  // echo request to response
  memcpy(response_buf, request_, request_len_);

  *response_     = response_buf;
  *response_len_ = request_len_;

  printf("Method   : %s called\n", method_);
  printf("Request  : %s\n",        request_);
  printf("Response : %s\n",       (char*)(*response_));
  printf("\n");

  // return success
  return 42;
}

int main()
{
  ECAL_HANDLE hserver = 0;

  // initialize eCAL API
  eCAL_Initialize("minimal server c", eCAL_Init_Default);

  // create server "service1"
  hserver = eCAL_Server_Create("service1");

  // add method callback for method "echo"
  eCAL_Server_AddMethodCallback(hserver, "echo", "", "", OnMethodCallback, 0);

  // idle
  while (eCAL_Ok())
  {
    // sleep 100 ms
    eCAL_Process_SleepMS(100);
  }

  // destroy server "service1"
  eCAL_Server_Destroy(hserver);

  // finalize eCAL API
  eCAL_Finalize();

  return(0);
}
