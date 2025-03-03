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

/**
 * @brief
**/

#include "ecal_clr_client.h"
#include "ecal_clr_common.h"

using namespace Continental::eCAL::Core;
using namespace Internal;

ServiceClient::ServiceClient(System::String^ service_name_)
{
  m_client = new ::eCAL::CServiceClient(StringToStlString(service_name_));
}

ServiceClient::~ServiceClient()
{
  delete m_client;
}

// What behavior is expected here
// should we throw an exception? is that more C# like?
List<ServiceClient::ServiceClientCallbackData^>^ ServiceClient::Call(System::String^ method_name_, array<Byte>^ request, const int rcv_timeout_)
{
  ::eCAL::ServiceResponseVecT responseVecT;

  if (m_client->CallWithResponse(StringToStlString(method_name_), ByteArrayToStlString(request), responseVecT, rcv_timeout_))
  {
    List<ServiceClientCallbackData^>^ rcv_Datas = gcnew List<ServiceClientCallbackData^>();

    for each(::eCAL::SServiceResponse response in responseVecT)
    {
      ServiceClientCallbackData^ rcv_data = gcnew ServiceClientCallbackData;
      rcv_data->call_state = static_cast<CallState>(response.call_state);
      rcv_data->error_msg = StlStringToString(response.error_msg);
      rcv_data->host_name = StlStringToString(response.server_id.service_id.host_name);
      rcv_data->method_name = StlStringToString(response.service_method_information.method_name);
      rcv_data->ret_state = response.ret_state;
      rcv_data->service_id = static_cast<long long>(response.server_id.service_id.entity_id);
      rcv_data->service_name = StlStringToString(response.server_id.service_name);
      rcv_data->response = StlStringToByteArray(response.response);
      rcv_Datas->Add(rcv_data);
    }
    return rcv_Datas;
  }
  else
  {
    return nullptr;
  }
}
