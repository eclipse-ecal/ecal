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
 * @file  ecal_clr_server.cpp
**/

#include "ecal_clr_server.h"
#include "ecal_clr_common.h"

using namespace Continental::eCAL::Core;
using namespace Internal;

ServiceServer::ServiceServer(System::String^ server_name_)
{
  m_serv = new ::eCAL::CServiceServer(StringToStlString(server_name_));
}

ServiceServer::~ServiceServer()
{
  delete m_serv;
}

bool ServiceServer::SetMethodCallback(String^ methodName, String^ reqType, String^ responseType, MethodCallback^ callback_)
{
  //if (m_callbacks == nullptr)
  //{
  //  m_serv_callback = gcnew servCallback(this, &ServiceServer::OnMethodCall);
  //  m_gch = GCHandle::Alloc(m_serv_callback);
  //  IntPtr ip = Marshal::GetFunctionPointerForDelegate(m_serv_callback);
  //  ::eCAL::SServiceMethodInformation method_info;
  //  method_info.method_name = StringToStlString(methodName);
  //  method_info.request_type.name = StringToStlString(reqType);
  //  method_info.response_type.name = StringToStlString(responseType);
  //  m_serv->SetMethodCallback(method_info, static_cast<stdcall_eCAL_MethodCallbackT>(ip.ToPointer()));
  //}
  //m_callbacks += callback_;
  //return true;

  return false;
}

bool ServiceServer::RemoveMethodCallback(String^ methodName, MethodCallback^ callback_)
{
  //if (m_callbacks == callback_)
  //{
  //  m_serv->RemoveMethodCallback(StringToStlString(methodName));
  //  m_gch.Free();
  //}
  //m_callbacks -= callback_;

  //return false;

  return false;
}

int ServiceServer::OnMethodCall(const ::eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
{
  array<Byte>^ result = m_callbacks(StlStringToString(method_info_.method_name), StlStringToString(method_info_.request_type.name), StlStringToString(method_info_.response_type.name), StlStringToByteArray(request_));
  response_ = ByteArrayToStlString(result);
  return 0;
}
