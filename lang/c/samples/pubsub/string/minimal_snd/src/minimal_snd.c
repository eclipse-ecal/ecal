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

int main()
{
  ECAL_HANDLE pub     = 0;
  char        snd_s[] = "HELLO WORLD FROM C";
  int         sent    = 0;

  // initialize eCAL API
  eCAL_Initialize("minimalc_snd", eCAL_Init_Default);

  // create publisher "Hello"
  pub = eCAL_Pub_New();
  eCAL_Pub_Create(pub, "Hello", "std::string", "base", "", 0);

  // send updates
  while(eCAL_Ok())
  {
    // send content
    sent = eCAL_Pub_Send(pub, snd_s, (int)strlen(snd_s), -1);
    if(sent <= 0) printf("Sending topic \"Hello\" failed !\n");
    else          printf("Published topic \"Hello\" with \"%s\"\n", snd_s);

    // sleep 100 ms
    eCAL_Process_SleepMS(100);
  }

  // finalize eCAL API
  eCAL_Finalize();

  return(0);
}
