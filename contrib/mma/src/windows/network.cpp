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

#include <iostream>
#include <list>
#include <regex>
#include <string>

#include "../include/windows/network.h"

Network::Network():
  first_time_function_call(true),
  old_timestamp(),
  send_net_speed(),
  receive_net_speed()
{
  pipe_command_ = "wmic nicconfig list brief/format:list";
}

Network::~Network()
{
}

const std::string Network::ConvertBSTRToStdString(const BSTR &bstr)
{
  int wslen = ::SysStringLen(bstr);
  int len = ::WideCharToMultiByte(CP_ACP, 0, (wchar_t*)bstr, wslen, NULL, 0, NULL, NULL);
  std::string dblstr(len, '\0');
  len = ::WideCharToMultiByte(CP_ACP, 0, (wchar_t*)bstr, wslen, &dblstr[0], len, NULL, NULL);
  if (len == 0) return std::string();
  return dblstr;
}

std::list<std::shared_ptr<Resource>> Network::GetResourceInfo(const HQUERY& /*h_query_*/, PDH_STATUS& /*pdhStatus*/, QueryManager& /*query_manager*/, std::string type /*= "2|3"*/)
{
  std::list<std::shared_ptr<Resource>> network_info_list;
  std::vector <std::string>            networks_ip;
  HRESULT                              hres;

  hres = CoInitializeEx(0, COINIT_MULTITHREADED);
  if (FAILED(hres))
  {
    std::cout << "Failed to initialize COM library. Error code = 0x" << std::hex << hres << std::endl;
    return network_info_list;
  }

  IWbemLocator *pointer_locator = nullptr;
  hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pointer_locator);
  if (FAILED(hres))
  {
    std::cout << "Failed to create IWbemLocator object." << " Err code = 0x" << std::hex << hres << std::endl;
    CoUninitialize();
    return network_info_list;
  }

  IWbemServices *pointer_services = nullptr;
  hres = pointer_locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, NULL, 0, 0, &pointer_services);
  if (FAILED(hres))
  {
    std::cout << "Could not connect. Error code = 0x" << std::hex << hres << std::endl;
    pointer_locator->Release();
    CoUninitialize();
    return network_info_list;
  }

  hres = CoSetProxyBlanket(pointer_services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
  if (FAILED(hres))
  {
    std::cout << "Could not set proxy blanket. Error code = 0x" << std::hex << hres << std::endl;
    pointer_services->Release();
    pointer_locator->Release();
    CoUninitialize();
    return network_info_list;
  }

  try
  {
    IEnumWbemClassObject* pointer_enumerator   = nullptr;
    IWbemClassObject*     pointer_class_object = nullptr;
    ULONG uReturn = 0;
    hres = pointer_services->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled = 'TRUE'"),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
      nullptr,
      &pointer_enumerator);
    while (pointer_enumerator != nullptr)
    {
      HRESULT hr = pointer_enumerator->Next(WBEM_INFINITE, 1, &pointer_class_object, &uReturn);
      if (0 == uReturn)
      {
        break;
      }
      VARIANT variant_properties;
      VariantInit(&variant_properties);
      hr = pointer_class_object->Get(L"IPAddress", 0, &variant_properties, 0, 0);
      if (variant_properties.vt != VT_NULL)
      {
        BSTR first_ip_address;
        static long index = 0;
        hr = ::SafeArrayGetElement(V_ARRAY(&variant_properties), &index, &first_ip_address);
        if (SUCCEEDED(hr))
        {
          networks_ip.push_back(ConvertBSTRToStdString(first_ip_address));
          ::SysFreeString(first_ip_address);
        }
      }
      VariantClear(&variant_properties);
      pointer_class_object->Release();
    }
    if(pointer_enumerator) pointer_enumerator->Release();

    hres = pointer_services->ExecQuery(
      bstr_t("WQL"),
      bstr_t("SELECT * FROM Win32_PerfRawData_Tcpip_NetworkInterface"),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
      NULL,
      &pointer_enumerator);

    if (FAILED(hres))
    {
      std::cout << "Query for operating system name failed." << " Error code = 0x" << std::hex << hres << std::endl;
      pointer_services->Release();
      pointer_locator->Release();
      CoUninitialize();
      return network_info_list;
    }

    std::time_t current_time = std::time(nullptr); //-V656
    if (true == first_time_function_call)
    {
      auto network_ip_size = networks_ip.size();
      receive_net_speed.resize(network_ip_size);          // stores the old value of call method
      send_net_speed.resize(network_ip_size);             // stores the old value of call method
      old_timestamp = std::time(nullptr);                // stores the old value of call method
      std::localtime(&old_timestamp);
    }
    else
    {
      std::localtime(&current_time);
    }
    auto iter = 0;
    for (auto ip : networks_ip)
    {
      pointer_enumerator->Next(WBEM_INFINITE, 1, &pointer_class_object, &uReturn);
      if (0 == uReturn)
      {
        break;
      }
      VARIANT variant_properties;
      VariantInit(&variant_properties);
      std::shared_ptr<Resource> network_buffer = std::make_shared<Network>();
      pointer_class_object->Get(L"Name", 0, &variant_properties, 0, 0);
      network_buffer->SetName(ConvertBSTRToStdString(variant_properties.bstrVal));
      network_buffer->SetIP_address(ip);
      VariantClear(&variant_properties);

      if (true == first_time_function_call)
      {
        network_buffer->SetSendNetworkSpeed(send_network_speed);
        network_buffer->SetReceiveNetworSpeed(receive_network_speed);
        network_info_list.push_back(network_buffer);

        pointer_class_object->Get(L"BytesReceivedPerSec", 0, &variant_properties, 0, 0);
        receive_net_speed[iter] = (ConvertBSTRToStdString(variant_properties.bstrVal));
        VariantClear(&variant_properties);

        pointer_class_object->Get(L"BytesSentPerSec", 0, &variant_properties, 0, 0);
        send_net_speed[iter] = (ConvertBSTRToStdString(variant_properties.bstrVal));
        VariantClear(&variant_properties);
      }
      else
      {
        pointer_class_object->Get(L"BytesReceivedPerSec", 0, &variant_properties, 0, 0);
        network_buffer->SetSendNetworkSpeed((std::stod(ConvertBSTRToStdString(variant_properties.bstrVal)) - std::stod(receive_net_speed[iter])) / (current_time - old_timestamp));
        receive_net_speed[iter] = (ConvertBSTRToStdString(variant_properties.bstrVal));
        VariantClear(&variant_properties);

        pointer_class_object->Get(L"BytesSentPerSec", 0, &variant_properties, 0, 0);
        network_buffer->SetReceiveNetworSpeed((std::stod(ConvertBSTRToStdString(variant_properties.bstrVal)) - std::stod(send_net_speed[iter])) / (current_time - old_timestamp));
        send_net_speed[iter] = (ConvertBSTRToStdString(variant_properties.bstrVal));
        VariantClear(&variant_properties);

        network_info_list.push_back(network_buffer);
      }

      pointer_class_object->Release();
      iter++;
    }
    pointer_enumerator->Release();

    old_timestamp = current_time;
    first_time_function_call = false;
  }
  catch (const std::exception& e)
  {
    std::cout << "Some network error orrcured: " << e.what() << std::endl;
  }
  pointer_services->Release();
  pointer_locator->Release();

  return network_info_list;
}
