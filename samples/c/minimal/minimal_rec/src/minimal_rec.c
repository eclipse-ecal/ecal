/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include <ecal/ecalc.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  ECAL_HANDLE sub     = 0;
  int         rcv     = 0;
  void*       rcv_buf = NULL;
  long long   time    = 0;

  // initialize eCAL API
  eCAL_Initialize(argc, argv, "minimalc_rec", eCAL_Init_Default);

  // create subscriber "Hello"
  sub = eCAL_Sub_New();
  eCAL_Sub_Create(sub, "Hello", "base:std::string", "", 0);

  // read updates
  while(eCAL_Ok())
  {
    // receive content with 100 ms timeout
    rcv = eCAL_Sub_Receive_Alloc(sub, &rcv_buf, &time, 100);
    if(rcv > 0)
    {
      // print content
      printf("Received topic \"Hello\" with \"%.*s\"\n", rcv, (char*)rcv_buf);

      // free buffer allocated by eCAL
      eCAL_FreeMem(rcv_buf);
    }
  }

  // destroy subscriber
  eCAL_Sub_Destroy(sub);

  // finalize eCAL API
  eCAL_Finalize(eCAL_Init_All);

  return(0);
}
