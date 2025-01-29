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

int main()
{
  ECAL_HANDLE sub         = 0;
  int         success     = 0;
  void*       rcv_buf     = NULL;
  int         rcv_buf_len = 0;
  long long   time        = 0;

  // initialize eCAL API
  eCAL_Initialize("minimalc_rec", eCAL_Init_Default);

  // create subscriber "Hello"
  sub = eCAL_Sub_New();
  eCAL_Sub_Create(sub, "Hello", "std::string", "base", "", 0);

  // read updates
  while(eCAL_Ok())
  {
    // receive content with 100 ms timeout
    success = eCAL_Sub_Receive_Buffer_Alloc(sub, &rcv_buf, &rcv_buf_len, &time, 100);
    if(success != 0)
    {
      // print content
      printf("Received topic \"Hello\" with \"%.*s\"\n", rcv_buf_len, (char*)rcv_buf);

      // free buffer allocated by eCAL
      eCAL_FreeMem(rcv_buf);
    }
  }

  // destroy subscriber
  eCAL_Sub_Destroy(sub);

  // finalize eCAL API
  eCAL_Finalize();

  return(0);
}
