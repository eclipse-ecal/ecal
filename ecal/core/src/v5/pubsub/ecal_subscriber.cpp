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
 * @brief  common data subscriber for eCAL
**/

#include <ecal/namespace.h>

#include <ecal/ecal.h>
#include <ecal/v5/ecal_subscriber.h>

#include "ecal_globals.h"
#include <pubsub/ecal_subscriber_impl.h>

#include <pubsub/config/builder/reader_attribute_builder.h>
#include "ecal_config_internal.h"

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace eCAL
{
  ECAL_CORE_NAMESPACE_V5
  {
    CSubscriber::CSubscriber() :
      m_subscriber_impl(nullptr)
    {
    }

    CSubscriber::CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Subscriber::Configuration& config_)
      : CSubscriber()
    {
      CSubscriber::Create(topic_name_, data_type_info_, config_);
    }

    CSubscriber::CSubscriber(const std::string& topic_name_, const Subscriber::Configuration& config_)
      : CSubscriber(topic_name_, SDataTypeInformation{}, config_)
    {
    }

    CSubscriber::~CSubscriber()
    {
      CSubscriber::Destroy();
    }

    CSubscriber::CSubscriber(CSubscriber&& rhs) noexcept :
      m_subscriber_impl(std::move(rhs.m_subscriber_impl))
    {
      rhs.m_subscriber_impl = nullptr;
    }

    CSubscriber& CSubscriber::operator=(CSubscriber&& rhs) noexcept
    {
      // Call destroy, to clean up the current state, then afterwards move all elements
      Destroy();

      m_subscriber_impl = std::move(rhs.m_subscriber_impl);

      rhs.m_subscriber_impl = nullptr;

      return *this;
    }

    bool CSubscriber::Create(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Subscriber::Configuration& config_)
    {
      if (m_subscriber_impl != nullptr) return(false);
      if (topic_name_.empty())     return(false);

      // create datareader
      m_subscriber_impl = std::make_shared<CSubscriberImpl>(data_type_info_, BuildReaderAttributes(topic_name_, config_, GetTransportLayerConfiguration(), GetRegistrationConfiguration()));

      // register datareader
      g_subgate()->Register(topic_name_, m_subscriber_impl);

      // we made it :-)
      return(true);
    }

    bool CSubscriber::Create(const std::string& topic_name_)
    {
      return Create(topic_name_, SDataTypeInformation{});
    }

    bool CSubscriber::Destroy()
    {
      if (m_subscriber_impl == nullptr) return(false);

      // remove receive callback
      RemReceiveCallback();

      // unregister datareader
      if (g_subgate() != nullptr) g_subgate()->Unregister(m_subscriber_impl->GetTopicName(), m_subscriber_impl);

#ifndef NDEBUG
      // log it
      eCAL::Logging::Log(Logging::log_level_debug1, std::string(m_subscriber_impl->GetTopicName() + "::CSubscriber::Destroy"));
#endif

      // destroy datareader
      m_subscriber_impl.reset();

      // we made it :-)
      return(true);
    }

    bool CSubscriber::SetID(const std::set<long long>& filter_ids_)
    {
      if (m_subscriber_impl == nullptr) return(false);
      m_subscriber_impl->SetFilterIDs(filter_ids_);
      return(true);
    }

    bool CSubscriber::SetAttribute(const std::string& attr_name_, const std::string& attr_value_)
    {
      if (m_subscriber_impl == nullptr) return false;
      return m_subscriber_impl->SetAttribute(attr_name_, attr_value_);
    }

    bool CSubscriber::ClearAttribute(const std::string& attr_name_)
    {
      if (m_subscriber_impl == nullptr) return false;
      return m_subscriber_impl->ClearAttribute(attr_name_);
    }

    bool CSubscriber::ReceiveBuffer(std::string& buf_, long long* time_ /* = nullptr */, int rcv_timeout_ /* = 0 */) const
    {
      if (m_subscriber_impl == nullptr) return(false);
      return(m_subscriber_impl->Read(buf_, time_, rcv_timeout_));
    }

    bool CSubscriber::AddReceiveCallback(ReceiveCallbackT callback_)
    {
      auto v6_callback = [callback_](const STopicId& topic_id_, const SDataTypeInformation&, const eCAL::SReceiveCallbackData& v6_callback_data)
        {
          eCAL::v5::SReceiveCallbackData v5_callback_data;
          // we all know the const_cast is evil, however, the old api didn't define the buffer as const.
          v5_callback_data.buf = const_cast<void*>(v6_callback_data.buffer);
          v5_callback_data.size = static_cast<long>(v6_callback_data.buffer_size);
          v5_callback_data.id = 0; // v6 callbacks do not communicate this data any more, hence it is always set to 0.
          v5_callback_data.time = v6_callback_data.send_timestamp;
          v5_callback_data.clock = v6_callback_data.send_clock;
          callback_(topic_id_.topic_name.c_str(), &v5_callback_data);
        };
      return AddReceiveCallback(v6_callback);
    }

    bool CSubscriber::AddReceiveCallback(eCAL::ReceiveCallbackT callback_)
    {
      if (m_subscriber_impl == nullptr) return(false);
      RemReceiveCallback();
      return(m_subscriber_impl->SetReceiveCallback(std::move(callback_)));
    }

    bool CSubscriber::RemReceiveCallback()
    {
      if (m_subscriber_impl == nullptr) return(false);
      return(m_subscriber_impl->RemoveReceiveCallback());
    }

    bool CSubscriber::AddEventCallback(eSubscriberEvent type_, SubEventCallbackT callback_)
    {
      if (m_subscriber_impl == nullptr) return(false);
      RemEventCallback(type_);
      return(m_subscriber_impl->SetEventCallback(type_, callback_));
    }

    bool CSubscriber::RemEventCallback(eSubscriberEvent type_)
    {
      if (m_subscriber_impl == nullptr) return(false);
      return(m_subscriber_impl->RemoveEventCallback(type_));
    }

    bool CSubscriber::IsPublished() const
    {
      if (m_subscriber_impl == nullptr) return(false);
      return(m_subscriber_impl->IsPublished());
    }

    size_t CSubscriber::GetPublisherCount() const
    {
      if (m_subscriber_impl == nullptr) return(0);
      return(m_subscriber_impl->GetPublisherCount());
    }

    std::string CSubscriber::GetTopicName() const
    {
      if (m_subscriber_impl == nullptr) return("");
      return(m_subscriber_impl->GetTopicName());
    }

    STopicId CSubscriber::GetId() const
    {
      if (m_subscriber_impl == nullptr) return{};
      return(m_subscriber_impl->GetId());
    }

    SDataTypeInformation CSubscriber::GetDataTypeInformation() const
    {
      if (m_subscriber_impl == nullptr) return(SDataTypeInformation{});
      return(m_subscriber_impl->GetDataTypeInformation());
    }

    std::string CSubscriber::Dump(const std::string& indent_ /* = "" */) const
    {
      std::stringstream out;

      out << indent_ << "----------------------" << '\n';
      out << indent_ << " class CSubscriber    " << '\n';
      out << indent_ << "----------------------" << '\n';
      out << indent_ << "DUMP NOT SUPPORTED ANYMORE" << '\n';
      out << indent_ << "----------------------" << '\n';
      out << '\n';

      return(out.str());
    }
  }
}
