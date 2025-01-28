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

#include <ecal/ecal.h>

#include "ecal_globals.h"
#include "ecal_subscriber_impl.h"

#include "config/builder/reader_attribute_builder.h"
#include "ecal_config_internal.h"

namespace eCAL
{
  CSubscriber::CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Subscriber::Configuration& config_)
  {
    // create subscriber implementation
    m_subscriber_impl = std::make_shared<CSubscriberImpl>(data_type_info_, BuildReaderAttributes(topic_name_, config_, GetTransportLayerConfiguration(), GetRegistrationConfiguration()));

    // register subscriber
    if (g_subgate() != nullptr) g_subgate()->Register(topic_name_, m_subscriber_impl);
  }

  CSubscriber::CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const SubEventCallbackT& event_callback_, const Subscriber::Configuration& config_) :
    CSubscriber(topic_name_, data_type_info_, config_)
  {
    // add event callback for all current event types
    m_subscriber_impl->SetEventIDCallback(event_callback_);
  }

  CSubscriber::~CSubscriber()
  {
    // could be already destroyed by move
    if (m_subscriber_impl == nullptr) return;

    // unregister subscriber
    if (g_subgate() != nullptr) g_subgate()->Unregister(m_subscriber_impl->GetTopicName(), m_subscriber_impl);
  }

  CSubscriber::CSubscriber(CSubscriber&& rhs) noexcept :
    m_subscriber_impl(std::move(rhs.m_subscriber_impl))
  {
  }

  CSubscriber& CSubscriber::operator=(CSubscriber&& rhs) noexcept
  {
    if (this != &rhs)
    {
      m_subscriber_impl = std::move(rhs.m_subscriber_impl);
    }
    return *this;
  }

  bool CSubscriber::SetReceiveCallback(ReceiveCallbackT callback_)
  {
    if (m_subscriber_impl == nullptr) return false;
    return m_subscriber_impl->SetReceiveCallback(std::move(callback_));
  }

  bool CSubscriber::RemoveReceiveCallback()
  {
    if (m_subscriber_impl == nullptr) return false;
    return m_subscriber_impl->RemoveReceiveCallback();
  }

  size_t CSubscriber::GetPublisherCount() const
  {
    if (m_subscriber_impl == nullptr) return 0;
    return m_subscriber_impl->GetPublisherCount();
  }

  std::string CSubscriber::GetTopicName() const
  {
    if (m_subscriber_impl == nullptr) return "";
    return m_subscriber_impl->GetTopicName();
  }

  STopicId CSubscriber::GetTopicId() const
  {
    if (m_subscriber_impl == nullptr) return{};
    return m_subscriber_impl->GetId();
  }

  SDataTypeInformation CSubscriber::GetDataTypeInformation() const
  {
    if (m_subscriber_impl == nullptr) return SDataTypeInformation{};
    return m_subscriber_impl->GetDataTypeInformation();
  }
}
