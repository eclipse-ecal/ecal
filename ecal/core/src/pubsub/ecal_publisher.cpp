/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
 * @brief  eCAL publisher interface
**/

#include <ecal/ecal.h>

#include "ecal_globals.h"
#include "ecal_publisher_impl.h"
#include "readwrite/ecal_writer_buffer_payload.h"

#include "config/builder/writer_attribute_builder.h"
#include "ecal_config_internal.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace eCAL
{
  CPublisher::CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_)
  {
    auto config = eCAL::GetConfiguration();
    config.publisher = config_;

    // create publisher implementation
    auto publisher_impl = std::make_shared<CPublisherImpl>(data_type_info_, BuildWriterAttributes(topic_name_, config));
    if (!publisher_impl) return;
    
    m_publisher_impl = publisher_impl;

    // register publisher
    auto pubgate = g_pubgate();
    if (pubgate) pubgate->Register(topic_name_, publisher_impl);
  }

  CPublisher::CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const PubEventCallbackT& event_callback_, const Publisher::Configuration& config_) :
    CPublisher(topic_name_, data_type_info_, config_)
  {
    auto publisher_impl = m_publisher_impl.lock();
    // add event callback for all current event types
    if (publisher_impl) publisher_impl->SetEventCallback(event_callback_);
  }

  CPublisher::~CPublisher()
  {
    auto publisher_impl = m_publisher_impl.lock();

    // unregister publisher
    auto pubgate = g_pubgate();
    if (pubgate && publisher_impl) pubgate->Unregister(publisher_impl->GetTopicName(), publisher_impl);
  }

  CPublisher::CPublisher(CPublisher&& rhs) noexcept
  {
    std::swap(m_publisher_impl, rhs.m_publisher_impl);
  }

  CPublisher& CPublisher::operator=(CPublisher&& rhs) noexcept
  {
    {
      auto publisher_impl = m_publisher_impl.lock();
      // clean-up existing m_publisher_impl before swapping with rhs
      auto pubgate = g_pubgate();
      if (pubgate && publisher_impl) pubgate->Unregister(publisher_impl->GetTopicName(), publisher_impl);
    }
    std::swap(m_publisher_impl, rhs.m_publisher_impl);
    return *this;
  }

  bool CPublisher::Send(const void* const buf_, const size_t len_, const long long time_ /* = DEFAULT_TIME_ARGUMENT */)
  {
    CBufferPayloadWriter payload{ buf_, len_ };
    return Send(payload, time_);
  }

  bool CPublisher::Send(CPayloadWriter& payload_, long long time_)
  {
    auto publisher_impl = m_publisher_impl.lock();
    if (!publisher_impl) return false;
    // in an optimization case the
     // publisher can send an empty package
     // or we do not have any subscription at all
     // then the data writer will only do some statistics
     // for the monitoring layer and return
    if (GetSubscriberCount() == 0)
    {
      publisher_impl->RefreshSendCounter();
      // we return false here to indicate that we did not really send something
      return false;
    }

    // send content via data writer layer
    const long long write_time = (time_ == DEFAULT_TIME_ARGUMENT) ? eCAL::Time::GetMicroSeconds() : time_;
    return publisher_impl->Write(payload_, write_time, 0);
  }

  bool CPublisher::Send(const std::string& payload_, long long time_)
  {
    return(Send(payload_.data(), payload_.size(), time_));
  }

  size_t CPublisher::GetSubscriberCount() const
  {
    auto publisher_impl = m_publisher_impl.lock();
    if (publisher_impl) return publisher_impl->GetSubscriberCount();
    return 0;
  }

  const std::string& CPublisher::GetTopicName() const
  {
    static const std::string empty_topic_name{};
    auto publisher_impl = m_publisher_impl.lock();
    if (publisher_impl) return publisher_impl->GetTopicName();
    return empty_topic_name;
  }

  const STopicId& CPublisher::GetTopicId() const
  {
    static const STopicId empty_topic_id{};
    auto publisher_impl = m_publisher_impl.lock();
    if (publisher_impl) return publisher_impl->GetTopicId();
    return empty_topic_id;
  }

  const SDataTypeInformation& CPublisher::GetDataTypeInformation() const
  {
    static const SDataTypeInformation empty_data_type_information{};
    auto publisher_impl = m_publisher_impl.lock();
    if (publisher_impl) return publisher_impl->GetDataTypeInformation();
    return empty_data_type_information;
  }
}
