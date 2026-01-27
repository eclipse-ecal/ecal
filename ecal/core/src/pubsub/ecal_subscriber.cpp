/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include <ecal/ecal.h>

#include "ecal_globals.h"
#include "ecal_subscriber_impl.h"

#include "config/builder/reader_attribute_builder.h"
#include "ecal_config_internal.h"

namespace eCAL
{
  CSubscriber::CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Subscriber::Configuration& config_)
  {
    auto config = eCAL::GetConfiguration();
    config.subscriber = config_;

    eCAL::SSubscriberGlobalContext global_context;
    
    if (auto globals = g_globals(); globals)
    {
      global_context.registration_provider = globals->registration_provider();
      global_context.log_provider          = globals->log_provider();
      global_context.shm_layer             = globals->shm_reader_layer();
      global_context.udp_layer             = globals->udp_reader_layer();
      global_context.tcp_layer             = globals->tcp_reader_layer();
    }
    

    // create subscriber implementation
    auto subscriber_impl = std::make_shared<CSubscriberImpl>(data_type_info_, BuildReaderAttributes(topic_name_, config), global_context);
    m_subscriber_impl = subscriber_impl;

    // register subscriber
    auto subgate = g_subgate();
    if (subgate) subgate->Register(topic_name_, subscriber_impl);
  }

  CSubscriber::CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const SubEventCallbackT& event_callback_, const Subscriber::Configuration& config_) :
    CSubscriber(topic_name_, data_type_info_, config_)
  {
    auto subscriber_impl = m_subscriber_impl.lock();
    // add event callback for all current event types
    if (subscriber_impl) subscriber_impl->SetEventCallback(event_callback_);
  }

  CSubscriber::~CSubscriber()
  {
    auto subscriber_impl = m_subscriber_impl.lock();

    // unregister subscriber
    auto subgate = g_subgate();
    if (subgate && subscriber_impl) subgate->Unregister(subscriber_impl->GetTopicName(), subscriber_impl);
  }

  CSubscriber::CSubscriber(CSubscriber&& rhs) noexcept
  {
    std::swap(m_subscriber_impl, rhs.m_subscriber_impl);
  }

  CSubscriber& CSubscriber::operator=(CSubscriber&& rhs) noexcept
  {
    {
      auto subscriber_impl = m_subscriber_impl.lock();
      // clean-up existing m_subscriber_impl before swapping with rhs
      auto subgate = g_subgate();
      if (subgate && subscriber_impl) subgate->Unregister(subscriber_impl->GetTopicName(), subscriber_impl);
    }
    std::swap(m_subscriber_impl, rhs.m_subscriber_impl);
    return *this;
  }

  void CSubscriber::SetReceiveCallback(ReceiveCallbackT callback_)
  {
    auto subscriber_impl = m_subscriber_impl.lock();
    if (subscriber_impl) static_cast<void>(subscriber_impl->SetReceiveCallback(callback_));
  }

  void CSubscriber::RemoveReceiveCallback()
  {
    auto subscriber_impl = m_subscriber_impl.lock();
    if (subscriber_impl) static_cast<void>(subscriber_impl->RemoveReceiveCallback());
  }

  size_t CSubscriber::GetPublisherCount() const
  {
    auto subscriber_impl = m_subscriber_impl.lock();
    if (subscriber_impl) return subscriber_impl->GetPublisherCount();
    return 0;
  }

  const std::string& CSubscriber::GetTopicName() const
  {
    auto subscriber_impl = m_subscriber_impl.lock();
    static const std::string empty_topic_name{};
    if (subscriber_impl) return subscriber_impl->GetTopicName();
    return empty_topic_name;
  }

  const STopicId& CSubscriber::GetTopicId() const
  {
    auto subscriber_impl = m_subscriber_impl.lock();
    static const STopicId empty_topic_id{};
    if (subscriber_impl) return subscriber_impl->GetTopicId();
    return empty_topic_id;
  }

  const SDataTypeInformation& CSubscriber::GetDataTypeInformation() const
  {
    auto subscriber_impl = m_subscriber_impl.lock();
    static const SDataTypeInformation empty_data_type_information{};
    if (subscriber_impl) return subscriber_impl->GetDataTypeInformation();
    return empty_data_type_information;
  }
}
