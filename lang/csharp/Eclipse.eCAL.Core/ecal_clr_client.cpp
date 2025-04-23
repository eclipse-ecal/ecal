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

#include <msclr/marshal_cppstd.h>

#include "ecal_clr_client.h"

#include "ecal_clr_common.h"
#include "ecal_clr_types_service.h"

#include <vector>

using namespace Eclipse::eCAL::Core;
using namespace Internal;

using namespace msclr::interop;

// Constructor
ServiceClient::ServiceClient(String^ serviceName, ServiceMethodInformationList^ methodInformationList)
{
  // Convert serviceName to std::string.
  std::string nativeServiceName = StringToStlString(serviceName);

  // Create an empty native set.
  ::eCAL::ServiceMethodInformationSetT nativeSet;
  
  // Only convert if methodInformationList is provided.
  if (methodInformationList != nullptr)
  {
    for each(ServiceMethodInformation^ mInfo in methodInformationList)
    {
      ::eCAL::SServiceMethodInformation nativeInfo;
      nativeInfo.method_name = StringToStlString(mInfo->MethodName);

      nativeInfo.request_type.name        = StringToStlString(mInfo->RequestType->Name);
      nativeInfo.request_type.encoding    = StringToStlString(mInfo->RequestType->Encoding);
      nativeInfo.request_type.descriptor  = ByteArrayToStlString(mInfo->RequestType->Descriptor);

      nativeInfo.response_type.name       = StringToStlString(mInfo->ResponseType->Name);
      nativeInfo.response_type.encoding   = StringToStlString(mInfo->ResponseType->Encoding);
      nativeInfo.response_type.descriptor = ByteArrayToStlString(mInfo->ResponseType->Descriptor);

      nativeSet.insert(nativeInfo);
    }
  }
  
  m_native_service_client = new ::eCAL::CServiceClient(nativeServiceName, nativeSet);
}

// Destructor
ServiceClient::~ServiceClient()
{
  this->!ServiceClient();
}

// Finalizer
ServiceClient::!ServiceClient()
{
  if(m_native_service_client != nullptr)
  {
    delete m_native_service_client;
    m_native_service_client = nullptr;
  }
}

// CallWithResponse: Blocking call that returns a List<ServiceResponse^>.
List<ServiceResponse^>^ ServiceClient::CallWithResponse(String^ methodName, array<Byte>^ request, int timeoutMs)
{
  std::string nativeMethod  = StringToStlString(methodName);
  std::string nativeRequest = ByteArrayToStlString(request);

  ::eCAL::ServiceResponseVecT nativeResponseVecT;
  if (m_native_service_client->CallWithResponse(nativeMethod, nativeRequest, nativeResponseVecT, timeoutMs))
  {
    List<ServiceResponse^>^ responseList = gcnew List<ServiceResponse^>();

    for each(::eCAL::SServiceResponse nativeResponse in nativeResponseVecT)
    {
      ServiceResponse^ response = gcnew ServiceResponse();
      response->CallState    = static_cast<CallState>(nativeResponse.call_state);
      response->ErrorMessage = StlStringToString(nativeResponse.error_msg);

      response->ServerId     = gcnew ServiceId(
        gcnew EntityId(
          nativeResponse.server_id.service_id.entity_id,
          nativeResponse.server_id.service_id.process_id,
          StlStringToString(nativeResponse.server_id.service_id.host_name)),
        StlStringToString(nativeResponse.server_id.service_name));

      response->MethodInformation = gcnew ServiceMethodInformation();
      response->MethodInformation->MethodName = StlStringToString(nativeResponse.service_method_information.method_name);

      response->MethodInformation->RequestType = gcnew DataTypeInformation(
        StlStringToString(nativeResponse.service_method_information.request_type.name),
        StlStringToString(nativeResponse.service_method_information.request_type.encoding),
        StlStringToByteArray(nativeResponse.service_method_information.request_type.descriptor));

      response->MethodInformation->ResponseType = gcnew DataTypeInformation(
        StlStringToString(nativeResponse.service_method_information.response_type.name),
        StlStringToString(nativeResponse.service_method_information.response_type.encoding),
        StlStringToByteArray(nativeResponse.service_method_information.response_type.descriptor));

      response->RetState = nativeResponse.ret_state;
      response->Response = StlStringToByteArray(nativeResponse.response);

      responseList->Add(response);
    }
    return responseList;
  }
  else
  {
    return gcnew List<ServiceResponse^>();
  }
}

// IsConnected: Check if at least one client instance is connected.
bool ServiceClient::IsConnected()
{
  return m_native_service_client->IsConnected();
}

// GetServiceId: Convert native SServiceId to managed ServiceId.
ServiceId^ ServiceClient::GetServiceId()
{
  ::eCAL::SServiceId nativeServiceId = m_native_service_client->GetServiceId();
  return gcnew ServiceId(
    gcnew EntityId(
      nativeServiceId.service_id.entity_id,
      nativeServiceId.service_id.process_id,
      StlStringToString(nativeServiceId.service_id.host_name)),
    StlStringToString(nativeServiceId.service_name));
}

// GetServiceName: Convert native service name to managed string.
String^ ServiceClient::GetServiceName()
{
  std::string nativeName = m_native_service_client->GetServiceName();
  return StlStringToString(nativeName);
}
