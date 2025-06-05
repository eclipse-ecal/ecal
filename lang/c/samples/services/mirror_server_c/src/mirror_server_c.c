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

void printCallbackInformation(const struct eCAL_SServiceMethodInformation* method_info_, const char* request_, size_t request_length_, const char* response_, size_t* response_length_)
{
   /*
    The data we get will not be \0 terminated. Hence we need to pass the size to printf.
  */
  printf("Method   : '%s' called in C\n", method_info_->method_name);
  printf("Request  : %.*s\n", (int)request_length_, request_);
  printf("Response : %.*s\n", (int)*response_length_, response_);
  printf("\n");
}

/*
  We define the callback function that will be called when a client calls the service method "echo".
  This callback will simply return the request as the response.
*/
int OnEchoCallback(const struct eCAL_SServiceMethodInformation* method_info_, const void* request_, size_t request_length_, void** response_, size_t* response_length_, void* user_argument_)
{
  (void)user_argument_;

  /*
    In order pass the server response properly to the callback API, the underlying memory needs to be allocated 
    with eCAL_Malloc(). The allocation via eCAL_Malloc() is required as the internal memory handler frees the
    resevered memory after callback execution.
  */
  *response_ = eCAL_Malloc(request_length_);

  /*
    In case of a failure, the value that response_ points to, remains NULL.
  */
  if (*response_ == NULL) return -1; // memory allocation failed

  /*
    The length of response buffer needs to be set accordingly
  */
  *response_length_ = request_length_;
  /*
    In this example the entire request buffer will be copied over to the response buffer.
  */
  memcpy(*response_, request_, request_length_);

  printCallbackInformation(method_info_, (const char*)request_, request_length_, (const char*)(*response_), response_length_);

  return 0;
}

/* 
  This callback will be called when a client calls the service method "reverse".
  It will return the request in reverse order as the response.
*/
int OnReverseCallback(const struct eCAL_SServiceMethodInformation* method_info_, const void* request_, size_t request_length_, void** response_, size_t* response_length_, void* user_argument_)
{
  (void)user_argument_;

  /* 
    In order pass the server response properly to the callback API, the underlying memory needs to be allocated 
    with eCAL_Malloc(). The allocation via eCAL_Malloc() is required as the internal memory handler frees the
    resevered memory after callback execution.
  */
  *response_ = eCAL_Malloc(request_length_);

  /*
    In case of a failure, the value that response_ points to, remains NULL.
  */
  if (*response_ == NULL) return -1; // memory allocation failed

  /*
    The length of response buffer needs to be set accordingly
  */
  *response_length_ = request_length_;

  const char* request = (const char*)request_;
  char* response = (char*)(*response_);

  for (size_t i = 0; i < request_length_; ++i) {
    response[i] = request[request_length_ - 1 - i];
  }

  printCallbackInformation(method_info_, (const char*)request_, request_length_, (const char*)(*response_), response_length_);

  return 0;
}

int main()
{
  printf("------------------\n");
  printf(" C: Mirror Server\n");
  printf("------------------\n");

  /*
    Here we already create the objects we want to work with. 
    Later we will create the server handle and fill the ServiceMethodInformation struct with the required information.
  */
  eCAL_ServiceServer *mirror_server;
  struct eCAL_SServiceMethodInformation echo_method_information;
  struct eCAL_SServiceMethodInformation reverse_method_information;

  /*
    As always: initialize the eCAL API and give your process a name.
  */
  eCAL_Initialize("mirror server c", NULL, NULL);

  printf("eCAL %s (%s)\n", eCAL_GetVersionString(), eCAL_GetVersionDateString());
  eCAL_Process_SetState(eCAL_Process_eSeverity_healthy, eCAL_Process_eSeverityLevel_level1, "I feel good!");

  /*
    Now we create the mirror server and give it the name "mirror".
  */
  mirror_server = eCAL_ServiceServer_New("mirror", NULL);

  /*
    The server will have two methods: "echo" and "reverse".
    To set a callback, we need to set a ServiceMethodInformation struct as well as the callback function.
    In our example we will just set the method name of the struct and leave the other two fields empty.
  */
  memset(&echo_method_information, 0, sizeof(struct eCAL_SServiceMethodInformation));
  echo_method_information.method_name = "echo";
  eCAL_ServiceServer_SetMethodCallback(mirror_server, &echo_method_information, OnEchoCallback, NULL);

  memset(&reverse_method_information, 0, sizeof(struct eCAL_SServiceMethodInformation));
  reverse_method_information.method_name = "reverse";
  eCAL_ServiceServer_SetMethodCallback(mirror_server, &reverse_method_information, OnReverseCallback, NULL);

  /*
    Now we will go in an infinite loop, to wait for incoming service calls that will be handled with the callbacks.
  */
  while (eCAL_Ok())
  {
    eCAL_Process_SleepMS(500);
  }

  /*
    When finished, we need to delete the server handle to clean up properly.
  */
  eCAL_ServiceServer_Delete(mirror_server);

  /*
    After we are done, as always, finalize the eCAL API.
  */
  eCAL_Finalize();

  return 0;
}
