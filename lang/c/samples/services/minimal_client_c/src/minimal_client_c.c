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
  // the client handle
  ECAL_HANDLE hclient = 0;

  // initialize eCAL API
  eCAL_Initialize("minimal client c", eCAL_Init_Default);

  // create client for "service1"
  hclient = eCAL_Client_Create("service1");

  // call service method
  while (eCAL_Ok())
  {
    struct SServiceResponseC service_response;
    char                     request[]                 = "HELLO";
    char                     response[sizeof(request)] = { 0 };
    // call method "echo"
    printf("Calling service1:echo ..\n");
    if (eCAL_Client_Call_Wait(hclient, "echo", request, sizeof(request), -1, &service_response, &response, sizeof(response)))
    {
      // process response
      switch (service_response.call_state)
      {
      case call_state_executed:
        printf("Method 'echo' executed. Response : ");
        printf("%s", response);
        printf("\n\n");
        break;
      case call_state_failed:
        printf("Method 'echo' failed. Error : ");
        printf("%s", service_response.error_msg);
        printf("\n\n");
        break;
      default:
        break;
      }
    }
    else
    {
      printf("Service / method not found :-(\n\n");
    }

    // sleep a second
    eCAL_Process_SleepMS(1000);
  }

  // destroy client for "service1"
  eCAL_Client_Destroy(hclient);

  // finalize eCAL API
  eCAL_Finalize();

  return(0);
}
