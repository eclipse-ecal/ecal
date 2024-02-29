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
 * @file   ecal_publisher_cimpl.cpp
 * @brief  eCAL publisher c interface
**/

#include <ecal/ecal.h>
#include <ecal/cimpl/ecal_publisher_cimpl.h>

#include "ecal_common_cimpl.h"

#include <mutex>

#if ECAL_CORE_PUBLISHER
namespace
{
  std::recursive_mutex g_pub_event_callback_mtx; // NOLINT(*-avoid-non-const-global-variables)
  void g_pub_event_callback(const char* topic_name_, const struct eCAL::SPubEventCallbackData* data_, const PubEventCallbackCT callback_, void* par_)
  {
    const std::lock_guard<std::recursive_mutex> lock(g_pub_event_callback_mtx);
    SPubEventCallbackDataC data{};
    data.type      = data_->type;
    data.time      = data_->time;
    data.clock     = data_->clock;
    data.tid       = data_->tid.c_str();
    data.tname     = data_->tdatatype.name.c_str();
    data.tencoding = data_->tdatatype.encoding.c_str();
    data.tdesc     = data_->tdatatype.descriptor.c_str();
    callback_(topic_name_, &data, par_);
  }
}

extern "C"
{
  ECALC_API ECAL_HANDLE eCAL_Pub_New()
  {
    auto* pub = new eCAL::CPublisher; // NOLINT(*-owning-memory)
    return(pub);
  }

  ECALC_API int eCAL_Pub_Create(ECAL_HANDLE handle_, const char* topic_name_, const char* topic_type_name_, const char* topic_type_encoding_, const char* topic_desc_, int topic_desc_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    const eCAL::SDataTypeInformation topic_info = { topic_type_name_, topic_type_encoding_, std::string(topic_desc_, static_cast<size_t>(topic_desc_len_)) };
    if (!pub->Create(topic_name_, topic_info)) return(0);
    return(1);
  }

  ECALC_API int eCAL_Pub_Destroy(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    delete pub; // NOLINT(*-owning-memory)
    return(1);
  }

  ECALC_API int eCAL_Pub_SetAttribute(ECAL_HANDLE handle_, const char* attr_name_, int attr_name_len_, const char* attr_value_, int attr_value_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (pub->SetAttribute(std::string(attr_name_, static_cast<size_t>(attr_name_len_)), std::string(attr_value_, static_cast<size_t>(attr_value_len_)))) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_ClearAttribute(ECAL_HANDLE handle_, const char* attr_name_, int attr_name_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (pub->ClearAttribute(std::string(attr_name_, static_cast<size_t>(attr_name_len_)))) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_ShareType(ECAL_HANDLE handle_, int state_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    pub->ShareType(state_ != 0);
    return(1);
  }

  ECALC_API int eCAL_Pub_ShareDescription(ECAL_HANDLE handle_, int state_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    pub->ShareDescription(state_ != 0);
    return(1);
  }

  ECALC_API int eCAL_Pub_SetID(ECAL_HANDLE handle_, long long id_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (pub->SetID(id_)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_IsSubscribed(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (pub->IsSubscribed()) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_Send(ECAL_HANDLE handle_, const void* const buf_, int buf_len_, long long time_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    const size_t ret = pub->Send(buf_, static_cast<size_t>(buf_len_), time_);
    if (static_cast<int>(ret) == buf_len_)
    {
      return(buf_len_);
    }
    return(0);
  }

  ECALC_API int eCAL_Pub_AddEventCallback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_, PubEventCallbackCT callback_, void* par_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    auto callback = std::bind(g_pub_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
    if (pub->AddEventCallback(type_, callback)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_RemEventCallback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (pub->RemEventCallback(type_)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_Dump(ECAL_HANDLE handle_, void* buf_, int buf_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* pub = static_cast<eCAL::CPublisher*>(handle_);
    const std::string dump = pub->Dump();
    if (!dump.empty())
    {
      return(CopyBuffer(buf_, buf_len_, dump));
    }
    return(0);
  }
}
#endif // ECAL_CORE_PUBLISHER
