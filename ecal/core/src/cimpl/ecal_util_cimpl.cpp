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

/**
 * @file   ecal_util_cimpl.cpp
 * @brief  eCAL utility c interface
**/

#include <ecal/ecal.h>
#include <ecal/cimpl/ecal_util_cimpl.h>

#include "ecal_common_cimpl.h"

extern "C"
{
#if ECAL_CORE_MONITORING
  ECALC_API void eCAL_Util_ShutdownUnitName(const char* unit_name_)
  {
    const std::string unit_name = unit_name_;
    eCAL::Util::ShutdownProcess(unit_name);
  }

  ECALC_API void eCAL_Util_ShutdownProcessID(int process_id_)
  {
    eCAL::Util::ShutdownProcess(process_id_);
  }

  ECALC_API void eCAL_Util_ShutdownProcesses()
  {
    eCAL::Util::ShutdownProcesses();
  }

  ECALC_API void eCAL_Util_ShutdownCore()
  {
    eCAL::Util::ShutdownCore();
  }
#endif // ECAL_CORE_MONITORING

  ECALC_API void eCAL_Util_EnableLoopback(int state_)
  {
    eCAL::Util::EnableLoopback(state_ != 0);
  }

  ECALC_API int eCAL_Util_GetTopicTypeName(const char* topic_name_, void* topic_type_, int topic_type_len_)
  {
    if (topic_name_ == nullptr) return(0);
    if (topic_type_ == nullptr) return(0);
    eCAL::SDataTypeInformation topic_info;
    if (eCAL::Util::GetTopicDataTypeInformation(topic_name_, topic_info))
    {
      return(CopyBuffer(topic_type_, topic_type_len_, topic_info.name));
    }
    return(0);
  }

  ECALC_API int eCAL_Util_GetTopicEncoding(const char* topic_name_, void* topic_encoding_, int topic_encoding_len_)
  {
    if (topic_name_ == nullptr)     return(0);
    if (topic_encoding_ == nullptr) return(0);
    eCAL::SDataTypeInformation topic_info;
    if (eCAL::Util::GetTopicDataTypeInformation(topic_name_, topic_info))
    {
      return(CopyBuffer(topic_encoding_, topic_encoding_len_, topic_info.encoding));
    }
    return(0);
  }

  ECALC_API int eCAL_Util_GetTopicDescription(const char* topic_name_, void* topic_desc_, int topic_desc_len_)
  {
    if (topic_name_ == nullptr) return(0);
    if (topic_desc_ == nullptr) return(0);
    eCAL::SDataTypeInformation topic_info;
    if (eCAL::Util::GetTopicDataTypeInformation(topic_name_, topic_info))
    {
      return(CopyBuffer(topic_desc_, topic_desc_len_, topic_info.descriptor));
    }
    return(0);
  }

  ECALC_API int eCAL_Util_GetServiceRequestTypeName(const char* service_name_, const char* method_name_, void* req_type_, int req_type_len_)
  {
    if (service_name_ == nullptr) return(0);
    if (method_name_ == nullptr)  return(0);
    if (req_type_ == nullptr)     return(0);
    std::string req_type;
    std::string resp_type;
    if (eCAL::Util::GetServiceTypeNames(service_name_, method_name_, req_type, resp_type))
    {
      return(CopyBuffer(req_type_, req_type_len_, req_type));
    }
    return 0;
  }

  ECALC_API int eCAL_Util_GetServiceResponseTypeName(const char* service_name_, const char* method_name_, void* resp_type_, int resp_type_len_)
  {
    if (service_name_ == nullptr) return(0);
    if (method_name_ == nullptr)  return(0);
    if (resp_type_ == nullptr)    return(0);
    std::string req_type;
    std::string resp_type;
    if (eCAL::Util::GetServiceTypeNames(service_name_, method_name_, req_type, resp_type))
    {
      return(CopyBuffer(resp_type_, resp_type_len_, resp_type));
    }
    return 0;
  }

  ECALC_API int eCAL_Util_GetServiceRequestDescription(const char* service_name_, const char* method_name_, void* req_desc_, int req_desc_len_)
  {
    if (service_name_ == nullptr) return(0);
    if (method_name_ == nullptr)  return(0);
    if (req_desc_ == nullptr)     return(0);
    std::string req_desc;
    std::string resp_desc;
    if (eCAL::Util::GetServiceDescription(service_name_, method_name_, req_desc, resp_desc))
    {
      return(CopyBuffer(req_desc_, req_desc_len_, req_desc));
    }
    return 0;
  }

  ECALC_API int eCAL_Util_GetServiceResponseDescription(const char* service_name_, const char* method_name_, void* resp_desc_, int resp_desc_len_)
  {
    if (service_name_ == nullptr) return(0);
    if (method_name_ == nullptr)  return(0);
    if (resp_desc_ == nullptr)    return(0);
    std::string req_desc;
    std::string resp_desc;
    if (eCAL::Util::GetServiceDescription(service_name_, method_name_, req_desc, resp_desc))
    {
      return(CopyBuffer(resp_desc_, resp_desc_len_, resp_desc));
    }
    return 0;
  }
}
