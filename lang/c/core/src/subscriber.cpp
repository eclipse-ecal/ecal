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
 * @file   subscriber.cpp
 * @brief  eCAL subscriber c interface
**/

#include <ecal/ecal.h>
#include <ecal/v5/ecal_subscriber.h>
#include <ecal_c/pubsub/subscriber.h>

#include "common.h"

#include <mutex>

#if ECAL_CORE_SUBSCRIBER
namespace
{
  eCAL_Subscriber_Event enum_class_to_enum(eCAL::eSubscriberEvent cpp_event_) {
    switch (cpp_event_) {
    case eCAL::eSubscriberEvent::none:                   return sub_event_none;
    case eCAL::eSubscriberEvent::connected:              return sub_event_connected;
    case eCAL::eSubscriberEvent::disconnected:           return sub_event_disconnected;
    case eCAL::eSubscriberEvent::dropped:                return sub_event_dropped;
    default:                                             return sub_event_none;
    }
  }

  eCAL::eSubscriberEvent enum_to_enum_class(eCAL_Subscriber_Event c_event_) {
    switch (c_event_) {
    case sub_event_none:                   return eCAL::eSubscriberEvent::none;
    case sub_event_connected:              return eCAL::eSubscriberEvent::connected;
    case sub_event_disconnected:           return eCAL::eSubscriberEvent::disconnected;
    case sub_event_dropped:                return eCAL::eSubscriberEvent::dropped;
    default:                               return eCAL::eSubscriberEvent::none;
    }
  }

  std::recursive_mutex g_sub_receive_callback_mtx; // NOLINT(*-avoid-non-const-global-variables)
  void g_sub_receive_callback(const char* topic_name_, const struct eCAL::v5::SReceiveCallbackData* data_, const ReceiveCallbackCT callback_, void* par_)
  {
    const std::lock_guard<std::recursive_mutex> lock(g_sub_receive_callback_mtx);
    SReceiveCallbackDataC data{};
    data.buf   = data_->buf;
    data.size  = data_->size;
    data.id    = data_->id;
    data.time  = data_->time;
    data.clock = data_->clock;
    callback_(topic_name_, &data, par_);
  }

  std::recursive_mutex g_sub_event_callback_mtx; // NOLINT(*-avoid-non-const-global-variables)
  void g_sub_event_callback(const char* topic_name_, const struct eCAL::v5::SSubEventCallbackData* data_, const SubEventCallbackCT callback_, void* par_)
  {
    const std::lock_guard<std::recursive_mutex> lock(g_sub_event_callback_mtx);
    SSubEventCallbackDataC data{};
    data.type       = enum_class_to_enum(data_->type);
    data.time       = data_->time;
    data.clock      = data_->clock;
    data.topic_id   = data_->tid.c_str();
    data.topic_name = data_->tdatatype.name.c_str();
    data.tencoding  = data_->tdatatype.encoding.c_str();
    data.tdesc      = data_->tdatatype.descriptor.c_str();
    callback_(topic_name_, &data, par_);
  }
}

extern "C"
{
  ECALC_API ECAL_HANDLE eCAL_Sub_New()
  {
    auto* sub = new eCAL::v5::CSubscriber; // NOLINT(*-owning-memory)
    return(sub);
  }

  ECALC_API int eCAL_Sub_Create(ECAL_HANDLE handle_, const char* topic_name_, const char* topic_type_name_, const char* topic_type_encoding_, const char* topic_desc_, int topic_desc_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    const eCAL::SDataTypeInformation topic_info = { topic_type_name_, topic_type_encoding_, std::string(topic_desc_, static_cast<size_t>(topic_desc_len_)) };
    if (!sub->Create(topic_name_, topic_info)) return(0);
    return(1);
  }

  ECALC_API int eCAL_Sub_Destroy(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    delete sub; // NOLINT(*-owning-memory)
    return(1);
  }

  ECALC_API int eCAL_Sub_SetID(ECAL_HANDLE handle_, const long long* id_array_, const int id_num_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    std::set<long long> id_set;
    if (id_array_ != nullptr)
    {
      for (size_t i = 0; i < static_cast<size_t>(id_num_); ++i)
      {
        id_set.insert(id_array_[i]);
      }
    }
    if (sub->SetID(id_set)) return(1);
    return(1);
  }

  ECALC_API int eCAL_Sub_SetAttribute(ECAL_HANDLE handle_, const char* attr_name_, int attr_name_len_, const char* attr_value_, int attr_value_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    if (sub->SetAttribute(std::string(attr_name_, static_cast<size_t>(attr_name_len_)), std::string(attr_value_, static_cast<size_t>(attr_value_len_)))) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_ClearAttribute(ECAL_HANDLE handle_, const char* attr_name_, int attr_name_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    if (sub->ClearAttribute(std::string(attr_name_, static_cast<size_t>(attr_name_len_)))) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_Receive_ToBuffer(ECAL_HANDLE handle_, void* buf_, int buf_len_, long long* time_, int rcv_timeout_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);

    std::string buf;
    if (sub->ReceiveBuffer(buf, time_, rcv_timeout_))
    {
      return(CopyBuffer(buf_, buf_len_, buf));
    }
    return(0);
  }

  ECALC_API int eCAL_Sub_Receive_Alloc(ECAL_HANDLE handle_, void** buf_, long long* time_, int rcv_timeout_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);

    std::string buf;
    if (sub->ReceiveBuffer(buf, time_, rcv_timeout_))
    {
      return(CopyBuffer(buf_, ECAL_ALLOCATE_4ME, buf)); // NOLINT(*-multi-level-implicit-pointer-conversion)
    }
    return(0);
  }

  ECALC_API int eCAL_Sub_Receive_Buffer_Alloc(ECAL_HANDLE handle_, void** buf_, int* buf_len_, long long* time_, int rcv_timeout_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);

    std::string buf;
    if (sub->ReceiveBuffer(buf, time_, rcv_timeout_))
    {
      CopyBuffer(buf_, ECAL_ALLOCATE_4ME, buf); // NOLINT(*-multi-level-implicit-pointer-conversion)
      if (buf_len_ != nullptr) *buf_len_ = static_cast<int>(buf.size());
      return(1);
    }
    return(0);
  }

  ECALC_API int eCAL_Sub_AddReceiveCallback(ECAL_HANDLE handle_, ReceiveCallbackCT callback_, void* par_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    auto callback = std::bind(g_sub_receive_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
    if (sub->AddReceiveCallback(callback)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_RemReceiveCallback(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    if (sub->RemReceiveCallback()) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_AddEventCallback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, SubEventCallbackCT callback_, void* par_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    auto callback = std::bind(g_sub_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
    if (sub->AddEventCallback(enum_to_enum_class(type_), callback)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_RemEventCallback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    if (sub->RemEventCallback(enum_to_enum_class(type_))) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_GetTypeName(ECAL_HANDLE handle_, void* buf_, int buf_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    const eCAL::SDataTypeInformation datatype_info = sub->GetDataTypeInformation();
    const int buffer_len = CopyBuffer(buf_, buf_len_, datatype_info.name);
    if (buffer_len != static_cast<int>(datatype_info.name.size()))
    {
      return(0);
    }
    else
    {
      return(buffer_len);
    }
  }

  ECALC_API int eCAL_Sub_GetEncoding(ECAL_HANDLE handle_, void* buf_, int buf_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    const eCAL::SDataTypeInformation datatype_info = sub->GetDataTypeInformation();
    const int buffer_len = CopyBuffer(buf_, buf_len_, datatype_info.encoding);
    if (buffer_len != static_cast<int>(datatype_info.encoding.size()))
    {
      return(0);
    }
    else
    {
      return(buffer_len);
    }
  }

  ECALC_API int eCAL_Sub_GetDescription(ECAL_HANDLE handle_, void* buf_, int buf_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    const eCAL::SDataTypeInformation datatype_info = sub->GetDataTypeInformation();
    const int buffer_len = CopyBuffer(buf_, buf_len_, datatype_info.descriptor);
    if (buffer_len != static_cast<int>(datatype_info.descriptor.size()))
    {
      return(0);
    }
    else
    {
      return(buffer_len);
    }
  }

  ECALC_API int eCAL_Sub_Dump(ECAL_HANDLE handle_, void* buf_, int buf_len_)
  {
    if (handle_ == nullptr) return(0);
    auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
    const std::string dump = sub->Dump();
    if (!dump.empty())
    {
      return(CopyBuffer(buf_, buf_len_, dump));
    }
    return(0);
  }
}
#endif // ECAL_CORE_SUBSCRIBER
