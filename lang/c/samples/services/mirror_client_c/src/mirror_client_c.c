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
#include <ecal_c/service/client.h>

#include <stdio.h>  // printf()
#include <string.h> // strlen
#include <stdlib.h> // free

/*
  Helper function to print the service response.
*/
void printServiceResponse(const struct eCAL_SServiceResponse* service_response)
{
  const char* call_state = NULL;
  
  switch (service_response->call_state)
  {
  case eCAL_eCallState_executed:
    call_state = "EXECUTED";
    break;
  case eCAL_eCallState_failed:
    call_state = "FAILED";
    break;
  default:
    call_state = "UNKNOWN";
    break;
  }

  printf("Received service response in C: %s\n", call_state);
  printf("Method    : %s\n", service_response->service_method_information.method_name);
  printf("Response  : %.*s\n", (int)service_response->response_length, (char*)service_response->response);
  printf("Server ID : %llu\n", service_response->server_id.service_id.entity_id);
  printf("Host      : %s\n", service_response->server_id.service_id.host_name);
  printf("\n");
}

/*
  Callback function that will be executed when we receive a response from a server.
*/
void serviceResponseCallback(const struct eCAL_SServiceResponse* service_response, void* user_data)
{
  (void)user_data;

  printServiceResponse(service_response); 
}


int main()
{
  eCAL_ServiceClient* mirror_client;
  struct eCAL_SServiceMethodInformation* method_information_set;

  printf("------------------\n");
  printf(" C: MIRROR CLIENT\n");
  printf("------------------\n");

  /*
    As always: initialize the eCAL API and give your process a name.
  */
  eCAL_Initialize("mirror client c", NULL, NULL);

  printf("eCAL %s (%s)\n", eCAL_GetVersionString(), eCAL_GetVersionDateString());
  eCAL_Process_SetState(eCAL_Process_eSeverity_healthy, eCAL_Process_eSeverityLevel_level1, "Waiting for a service ...");

  /*
    Create a ServiceMethodInformation struct that contains the information about the service methods we want to call.
    In this case, we want to call the methods "echo" and "reverse".
  */
  method_information_set = (struct eCAL_SServiceMethodInformation*)eCAL_Malloc(2 * sizeof(struct eCAL_SServiceMethodInformation));
  if (method_information_set == NULL)
  {
    printf("Memory allocation failed.\n");
    return -1;
  }

  memset(method_information_set, 0, 2 * sizeof(struct eCAL_SServiceMethodInformation));
  method_information_set[0].method_name = "echo";
  method_information_set[1].method_name = "reverse";

  /*
    Create a client that connects to a "mirror" server.
    It may call the methods "echo" and "reverse".
  */
  mirror_client = eCAL_ServiceClient_New("mirror", method_information_set, 0, NULL);

  /*
    Wait until the client is connected to a server,
    so we don't call methods that are not available.
  */
  while (!eCAL_ServiceClient_IsConnected(mirror_client))
  {
    printf("Waiting for a service ...\n");
    eCAL_Process_SleepMS(1000);
  }

  /*
    Now that we are connected, we can set the process state to "healthy" and communicate the connection.
  */
  eCAL_Process_SetState(eCAL_Process_eSeverity_healthy, eCAL_Process_eSeverityLevel_level1, "Connected!");

  /*
    Allow alternating between the two methods "echo" and "reverse".
  */
  const char* methods[] = { "echo", "reverse" };
  size_t method_count = sizeof(methods) / sizeof(methods[0]);
  size_t i = 0;
  unsigned int calls_ok = 0;

  while (eCAL_Ok())
  {
    /* 
      Alternate between the two methods "echo" and "reverse".
      Create the request payload.
    */
    const char* method_name = methods[i++ % method_count];
    char request[] = "stressed";
    
    struct eCAL_ClientInstance** client_instances = eCAL_ServiceClient_GetClientInstances(mirror_client);
    calls_ok = client_instances != NULL && client_instances[0] != NULL;

    /*
      We iterate now over all client instances and call the methods by name.
      With this approach we have the option to filter out client instances that we don't want to call.
      If you want to call either way all instances, then you can use

      eCAL_ServiceClient_CallWithResponse(mirror_client, ...)
      eCAL_ServiceClient_CallWithCallback(mirror_client, ...)

      instead of the loop.
    */
    
    if (client_instances != NULL)
    {
      for (size_t j = 0; client_instances[j] != NULL; ++j)
      {
        /*
          Service call: blocking
          We leave the default timeout value (infinite) for the blocking call.
          You can change this for a specified timeout in ms.
        */
        struct eCAL_SServiceResponse* response = eCAL_ClientInstance_CallWithResponse(client_instances[j], method_name, request, strlen(request), NULL);
        if (response != NULL)
        {
          printServiceResponse(response);
          eCAL_Free(response);
        }
        else
        {
          printf("Method blocking call failed.\n");
          calls_ok = 0;
        }

        /*
          Service call: with callback
          The callback will be executed when the server has processed the request and sent a response.
          You can again set a timeout value for an internal waiting time. By default, we wait infinitely.
        */
        if(eCAL_ClientInstance_CallWithCallback(client_instances[j], method_name, request, strlen(request), serviceResponseCallback, NULL, NULL) != 0)
        {
          printf("Method callback call failed.\n");
          calls_ok = 0;
        }
      }

      /*
        After usage, free the memory from the client instances.
      */
      eCAL_ClientInstances_Delete(client_instances);
    }

    /*
      Now we set the process state according to the result of the service calls.
      You will see the state in the eCAL Monitor or the eCAL Sys application.
    */
    if (calls_ok != 0)
    {
      eCAL_Process_SetState(eCAL_Process_eSeverity_healthy, eCAL_Process_eSeverityLevel_level1, "Connected!");
    }
    else
    {
      eCAL_Process_SetState(eCAL_Process_eSeverity_critical, eCAL_Process_eSeverityLevel_level3, "Calls failed!");
    }

    eCAL_Process_SleepMS(1000);
  }

  /*
    Don't forget to delete the client handle.
  */
  eCAL_ServiceClient_Delete(mirror_client);

  /*
    After we are done, as always, finalize the eCAL API.
  */
  eCAL_Finalize();

  return 0;
}
