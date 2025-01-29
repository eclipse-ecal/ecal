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

void OnReceive(const char* topic_name_, const struct SReceiveCallbackDataC* data_, void* par_)
{
  // unused param
  (void)par_;
  printf("Received topic \"%s\" with ", topic_name_);
  printf("\"%.*s\"\n", (int)(data_->size), (char*)(data_->buf));
}

int main()
{
  ECAL_HANDLE sub = 0;

  // initialize eCAL API
  eCAL_Initialize("minimalc_rec_cb", eCAL_Init_Default);

  // create subscriber "Hello"
  sub = eCAL_Sub_New();
  eCAL_Sub_Create(sub, "Hello", "std::string", "base", "", 0);

  // add callback
  eCAL_Sub_AddReceiveCallback(sub, OnReceive, NULL);

  // idle main thread
  while(eCAL_Ok())
  {
    // sleep 100 ms
    eCAL_Process_SleepMS(100);
  }

  // destroy subscriber
  eCAL_Sub_Destroy(sub);

  // finalize eCAL API
  eCAL_Finalize();

  return(0);
}
