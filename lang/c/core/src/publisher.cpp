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
 * @file   publisher.cpp
 * @brief  eCAL publisher c interface
**/

#include <ecal/ecal.h>
#include <ecal/v5/ecal_publisher.h>
#include <ecal_c/pubsub/publisher.h>

#include "common.h"
#include "configuration.h"

#include <algorithm>
#include <cassert>

#if ECAL_CORE_PUBLISHER
namespace
{
  void Assign_SPubEventCallbackData(struct eCAL_SPubEventCallbackData* pub_event_callback_data_c_, const eCAL::SPubEventCallbackData& pub_event_callback_data_)
  {
    static const std::map<eCAL::ePublisherEvent, eCAL_ePublisherEvent> publisher_event_map
    {
        {eCAL::ePublisherEvent::none, eCAL_ePublisherEvent_none},
        {eCAL::ePublisherEvent::connected, eCAL_ePublisherEvent_connected},
        {eCAL::ePublisherEvent::disconnected, eCAL_ePublisherEvent_disconnected},
        {eCAL::ePublisherEvent::dropped, eCAL_ePublisherEvent_dropped}
    };

    pub_event_callback_data_c_->event_type = publisher_event_map.at(pub_event_callback_data_.event_type);
    pub_event_callback_data_c_->event_time = pub_event_callback_data_.event_time;
    Assign_SDataTypeInformation(&pub_event_callback_data_c_->subscriber_datatype, pub_event_callback_data_.subscriber_datatype);
  }

  class PayloadWriter : public eCAL::CPayloadWriter
  {
  public:
    PayloadWriter(const struct eCAL_PayloadWriter* payload_writer_) : m_payload_writer(payload_writer_)
    {
    }

    bool WriteFull(void* buffer_, size_t size_) final
    {
      return !static_cast<bool>(m_payload_writer->WriteFull(buffer_, size_));
    }

    bool WriteModified(void* buffer_, size_t size_) final
    {
      if (m_payload_writer->WriteModified != NULL)
        return !static_cast<bool>(m_payload_writer->WriteModified(buffer_, size_));
      else
        return WriteFull(buffer_, size_);
    }

    size_t GetSize() final
    {
      return m_payload_writer->GetSize();
    }

  private:
    const struct eCAL_PayloadWriter* m_payload_writer;
  };
}

struct eCAL_Publisher
{
  eCAL::CPublisher* handle;
  eCAL_STopicId topic_id;
  eCAL_SDataTypeInformation data_type_info;
};

extern "C"
{
  ECALC_API eCAL_Publisher* eCAL_Publisher_New(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const eCAL_PubEventCallbackT pub_event_callback_, const struct eCAL_Publisher_Configuration* publisher_configuration_)
  {
    assert(topic_name_ != NULL);
    eCAL::SDataTypeInformation data_type_information;
    eCAL::PubEventCallbackT pub_event_callback;
    eCAL::Publisher::Configuration publisher_configuration = eCAL::GetPublisherConfiguration();

    if (data_type_information_ != NULL)
      Assign_SDataTypeInformation(data_type_information, data_type_information_);

    if (publisher_configuration_ != NULL)
      Assign_Publisher_Configuration(publisher_configuration, publisher_configuration_);

    if (pub_event_callback_ != NULL)
    {
      pub_event_callback = [pub_event_callback_](const eCAL::STopicId& topic_id_, const eCAL::SPubEventCallbackData& pub_event_callback_data_)
      {
        struct eCAL_STopicId topic_id_c;
        struct eCAL_SPubEventCallbackData pub_event_callback_data_c;

        Assign_STopicId(&topic_id_c, topic_id_);
        Assign_SPubEventCallbackData(&pub_event_callback_data_c, pub_event_callback_data_);
        pub_event_callback_(&topic_id_c, &pub_event_callback_data_c);
      };
    }

    return new eCAL_Publisher{ new eCAL::CPublisher(topic_name_, data_type_information, pub_event_callback, publisher_configuration) };
  }

  ECALC_API void eCAL_Publisher_Delete(eCAL_Publisher* publisher_)
  {
    assert(publisher_ != NULL);
    delete publisher_->handle;
    delete publisher_;
  }

  ECALC_API int eCAL_Publisher_Send(eCAL_Publisher* publisher_, const void* buffer_, size_t buffer_len_, const long long* timestamp_)
  {
    assert(publisher_ != NULL);
    return static_cast<int>(!publisher_->handle->Send(buffer_, buffer_len_, timestamp_ != NULL ? *timestamp_ : eCAL::CPublisher::DEFAULT_TIME_ARGUMENT));
  }

  ECALC_API int eCAL_Publisher_SendPayloadWriter(eCAL_Publisher* publisher_, const struct eCAL_PayloadWriter* payload_writer_, const long long* timestamp_)
  {
    assert(publisher_ != NULL && payload_writer_ != NULL);
    PayloadWriter payload_writer(payload_writer_);
    return static_cast<int>(!publisher_->handle->Send(payload_writer, timestamp_ != NULL ? *timestamp_ : eCAL::CPublisher::DEFAULT_TIME_ARGUMENT));
  }

  ECALC_API size_t eCAL_Publisher_GetSubscriberCount(eCAL_Publisher* publisher_)
  {
    assert(publisher_ != NULL);
    return publisher_->handle->GetSubscriberCount();
  }

  ECALC_API const char* eCAL_Publisher_GetTopicName(eCAL_Publisher* publisher_)
  {
    assert(publisher_ != NULL);
    return publisher_->handle->GetTopicName().c_str();
  }

  ECALC_API const struct eCAL_STopicId* eCAL_Publisher_GetTopicId(eCAL_Publisher* publisher_)
  {
    assert(publisher_ != NULL);
    Assign_STopicId(&publisher_->topic_id, publisher_->handle->GetTopicId());
    return &publisher_->topic_id;
  }

  ECALC_API const struct eCAL_SDataTypeInformation* eCAL_Publisher_GetDataTypeInformation(eCAL_Publisher* publisher_)
  {
    assert(publisher_ != NULL);
    Assign_SDataTypeInformation(&publisher_->data_type_info, publisher_->handle->GetDataTypeInformation());
    return &publisher_->data_type_info;
  }
}
#endif // ECAL_CORE_PUBLISHER
