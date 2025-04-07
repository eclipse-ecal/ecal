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

#include "ecal_clr_server.h"

#include "ecal_clr_common.h"
#include "ecal_clr_types.h"

#include <sstream>

using namespace Eclipse::eCAL::Core;
using namespace Internal;
using namespace System::Collections::Generic;

ServiceServer::ServiceServer(String^ serverName)
{
  m_native_service_server = new ::eCAL::CServiceServer(StringToStlString(serverName));

  m_managed_callbacks        = gcnew Dictionary<String^, MethodCallback^>();
  m_native_callbacks         = gcnew Dictionary<String^, servCallback^>();
  m_native_callbacks_handles = gcnew Dictionary<String^, GCHandle>();
}

ServiceServer::~ServiceServer()
{
  this->!ServiceServer();
}

ServiceServer::!ServiceServer()
{
  if (m_native_service_server != nullptr)
  {
    delete m_native_service_server;
    m_native_service_server = nullptr;
  }
  // Free any allocated GCHandles.
  for each(KeyValuePair<String^, GCHandle> kvp in m_native_callbacks_handles)
  {
    if (kvp.Value.IsAllocated)
      kvp.Value.Free();
  }
}

int ServiceServer::OnMethodCall(const ::eCAL::SServiceMethodInformation& methodInfo, const std::string& request, std::string& response)
{
  // Convert the native method name to managed.
  String^ methodName = StlStringToString(methodInfo.method_name);
  // Look up the managed callback for this method.
  MethodCallback^ managedCb = nullptr;
  if (!m_managed_callbacks->TryGetValue(methodName, managedCb))
  {
    return -1;
  }

  // Convert the native request and response types to managed.
  ServiceMethodInformation^ methodInfoManaged = gcnew ServiceMethodInformation();
  methodInfoManaged->MethodName   = methodName;
  methodInfoManaged->RequestType  = gcnew DataTypeInformation(
    StlStringToString(methodInfo.request_type.name),
    StlStringToString(methodInfo.request_type.encoding),
    StlStringToByteArray(methodInfo.request_type.descriptor));
  methodInfoManaged->ResponseType = gcnew DataTypeInformation(
    StlStringToString(methodInfo.response_type.name),
    StlStringToString(methodInfo.response_type.encoding),
    StlStringToByteArray(methodInfo.response_type.descriptor));
  array<Byte>^ reqData = StlStringToByteArray(request);

  // Invoke the managed delegate.
  array<Byte>^ respData = managedCb(methodInfoManaged, reqData);
  response = ByteArrayToStlString(respData);
  return 0;
}

bool ServiceServer::SetMethodCallback(ServiceMethodInformation^ methodInfo, MethodCallback^ callback)
{
  String^ methodName = methodInfo->MethodName;
  // Store the managed callback in the dictionary.
  m_managed_callbacks[methodName] = callback;

  // Create a native callback delegate.
  servCallback^ nativeCb = gcnew servCallback(this, &ServiceServer::OnMethodCall);
  IntPtr ip = Marshal::GetFunctionPointerForDelegate(nativeCb);

  // Build native SServiceMethodInformation from the managed ServiceMethodInformation.
  ::eCAL::SServiceMethodInformation nativeMethodInfo;
  nativeMethodInfo.method_name = StringToStlString(methodInfo->MethodName);
  if (methodInfo->RequestType != nullptr)
  {
    nativeMethodInfo.request_type.name = StringToStlString(methodInfo->RequestType->Name);
  }
  if (methodInfo->ResponseType != nullptr)
  {
    nativeMethodInfo.response_type.name = StringToStlString(methodInfo->ResponseType->Name);
  }

  // Register the native callback for this method.
  m_native_service_server->SetMethodCallback(nativeMethodInfo, static_cast<stdcall_eCAL_MethodCallbackT>(ip.ToPointer()));

  // Store the native callback and its GCHandle.
  m_native_callbacks[methodName] = nativeCb;
  GCHandle gch = GCHandle::Alloc(nativeCb);
  m_native_callbacks_handles[methodName] = gch;

  return true;
}

bool ServiceServer::RemoveMethodCallback(String^ methodName)
{
  // Unregister the native callback for this method.
  m_native_service_server->RemoveMethodCallback(StringToStlString(methodName));
  // Remove from our dictionaries.
  m_managed_callbacks->Remove(methodName);
  if (m_native_callbacks_handles->ContainsKey(methodName))
  {
    GCHandle gch = m_native_callbacks_handles[methodName];
    if (gch.IsAllocated)
      gch.Free();
    m_native_callbacks_handles->Remove(methodName);
  }
  m_native_callbacks->Remove(methodName);
  return true;
}
