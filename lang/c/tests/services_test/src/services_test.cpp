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

#include <gtest/gtest.h>

class services_test_c : public ::testing::Test 
{
    protected:
      eCAL_ServiceServer* server;
      eCAL_ServiceClient* client;
      const char* service_name = "services_test";

      void SetUp() override {
        // initialize eCAL API
        eCAL_Initialize("services_test_c", NULL, NULL);

        // create server
        server = eCAL_ServiceServer_New(service_name, NULL);

        // create client
        client = eCAL_ServiceClient_New(service_name, NULL, 0, NULL);
      }
  
      void TearDown() override {
        eCAL_ServiceServer_Delete(server);
        eCAL_ServiceClient_Delete(client);
        eCAL_Finalize();
      }
  };

int OnMethodCallback(const struct eCAL_SServiceMethodInformation* method_info_, const void* request_, size_t request_length_, void** response_, size_t* response_length_, void* user_argument_)
{
  (void)user_argument_;

  // In order pass the server response properly to the callback API, the underlying memory needs to be allocated 
  // with eCAL_Malloc(). The allocation via eCAL_Malloc() is required as the internal memory handler frees the
  // resevered memory after callback execution.
  *response_ = eCAL_Malloc(request_length_);

  // In case of a failure, the value that response_ points to, remains NULL.
  if (*response_ == NULL) return 1; // memory allocation failed

  // The length of response buffer needs to be set accordingly
  *response_length_ = request_length_;

  // In this example the entire request buffer will be copied over to the response buffer.
  memcpy(*response_, request_, request_length_);

  // Zero can be returned here as the callback has been successfully proceeded.
  return 0;
}

void ClientCallback(const struct eCAL_SServiceResponse* /*service_response_*/, void* user_argument_)
{
  int* cnt = (int*)user_argument_;
  (*cnt)++;
}

TEST_F(services_test_c, setup_test) 
{
  EXPECT_NE(nullptr, server);
  EXPECT_NE(nullptr, client);
}

TEST_F(services_test_c, server_set_and_remove_methodcallback) 
{
  eCAL_SServiceMethodInformation serviceInformation; 
  memset(&serviceInformation, 0, sizeof(struct eCAL_SServiceMethodInformation));
  const char* method_name = "on_method_callback";
  serviceInformation.method_name = method_name;

  EXPECT_EQ(0, eCAL_ServiceServer_SetMethodCallback(server, &serviceInformation, OnMethodCallback, NULL));
  EXPECT_EQ(0, eCAL_ServiceServer_RemoveMethodCallback(server, method_name));
  EXPECT_EQ(1, eCAL_ServiceServer_RemoveMethodCallback(server, method_name));
}

TEST_F(services_test_c, GetServiceName) 
{
  EXPECT_STREQ(service_name, eCAL_ServiceServer_GetServiceName(server));
  EXPECT_STREQ(service_name, eCAL_ServiceClient_GetServiceName(client));
}

TEST_F(services_test_c, IsConnected) 
{
  eCAL_Process_SleepMS(2000);
  EXPECT_NE(0, eCAL_ServiceServer_IsConnected(server));
  EXPECT_NE(0, eCAL_ServiceClient_IsConnected(client));
}

TEST_F(services_test_c, GetClientInstances) 
{
  EXPECT_NE(nullptr, eCAL_ServiceClient_GetClientInstances(client));
}

TEST_F(services_test_c, CallWithResponse)
{
  eCAL_SServiceMethodInformation serviceInformation; 
  memset(&serviceInformation, 0, sizeof(struct eCAL_SServiceMethodInformation));
  const char* method_name = "on_method_callback";
  serviceInformation.method_name = method_name;

  char                          request[] = "HELLO";
  struct eCAL_SServiceResponse* response = NULL;
  size_t                        response_length = 0;

  eCAL_ServiceServer_SetMethodCallback(server, &serviceInformation, OnMethodCallback, NULL);
  eCAL_Process_SleepMS(500);

  EXPECT_EQ(0, eCAL_ServiceClient_CallWithResponse(client, method_name, request, sizeof(request), &response, &response_length, NULL));
}

TEST_F(services_test_c, CallWithCallback)
{
  eCAL_SServiceMethodInformation serviceInformation; 
  memset(&serviceInformation, 0, sizeof(struct eCAL_SServiceMethodInformation));
  const char* method_name = "on_method_callback";
  serviceInformation.method_name = method_name;
  char request[] = "HELLO";
  int counter = 0;

  eCAL_ServiceServer_SetMethodCallback(server, &serviceInformation, OnMethodCallback, NULL);
  eCAL_Process_SleepMS(500);

  EXPECT_EQ(0, eCAL_ServiceClient_CallWithCallback(client, method_name, request, sizeof(request), ClientCallback, &counter, NULL));
  EXPECT_EQ(1, counter);
}

TEST_F(services_test_c, CallWithCallbackAsync)
{
  eCAL_SServiceMethodInformation serviceInformation; 
  memset(&serviceInformation, 0, sizeof(struct eCAL_SServiceMethodInformation));
  const char* method_name = "on_method_callback";
  serviceInformation.method_name = method_name;
  char request[] = "HELLO";
  int counter = 0;

  eCAL_ServiceServer_SetMethodCallback(server, &serviceInformation, OnMethodCallback, NULL);
  eCAL_Process_SleepMS(500);

  EXPECT_EQ(0, eCAL_ServiceClient_CallWithCallbackAsync(client, method_name, request, sizeof(request), ClientCallback, &counter));
  eCAL_Process_SleepMS(200);
  EXPECT_EQ(1, counter);
}