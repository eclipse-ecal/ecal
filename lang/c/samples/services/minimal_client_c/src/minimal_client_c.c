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

#include <stdio.h> // printf()

int main()
{
  // The client handle
  eCAL_ServiceClient *client;

  // Initialize eCAL API
  eCAL_Initialize("minimal client c", NULL, NULL);

  // Create client for "service1"
  client = eCAL_ServiceClient_New("service1", NULL, 0, NULL);

  // Call service method
  while (eCAL_Ok())
  {
    char                          request[] = "HELLO";
    struct eCAL_SServiceResponse* response = NULL;
    size_t                        response_length = 0;

    // Call method "echo"
    printf("Calling service1:echo ..\n");
    if (!eCAL_ServiceClient_CallWithResponse(client, "echo", request, sizeof(request), &response, &response_length, NULL))
    {
      for (size_t i = 0; i < response_length; ++i)
      {
        // Query responses from all servers that response to method "echo"
        switch (response[i].call_state)
        {
        case eCAL_eCallState_executed:
          printf("Method 'echo' executed. Response : ");
          printf("%s", (char*)response[i].response);
          printf("\n\n");
          break;
        case eCAL_eCallState_failed:
          printf("Method 'echo' failed. Error : ");
          printf("%s", response[i].error_msg);
          printf("\n\n");
          break;
        default:
          break;
        }
      }
    }
    else
    {
      printf("Service / method not found :-(\n\n");
    }

    // Memory of response array needs to be deallocated explicitly.
    eCAL_Free(response);

    // Sleep a second
    eCAL_Process_SleepMS(1000);
  }

  // Destroy client for "service1"
  eCAL_ServiceClient_Delete(client);

  // Finalize eCAL API
  eCAL_Finalize();

  return 0;
}
