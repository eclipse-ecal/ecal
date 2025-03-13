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
#include <ecal_c/pubsub/subscriber.h>

#include "common.h"
#include "configuration.h"

#include <cassert>

#if ECAL_CORE_SUBSCRIBER
namespace
{
  void Assign_SSubEventCallbackData(struct eCAL_SSubEventCallbackData* sub_event_callback_data_c_, const eCAL::SSubEventCallbackData& sub_event_callback_data_)
  {
    static const std::map<eCAL::eSubscriberEvent, eCAL_eSubscriberEvent> subscriber_event_map
    {
        {eCAL::eSubscriberEvent::none, eCAL_eSubscriberEvent_none},
        {eCAL::eSubscriberEvent::connected, eCAL_eSubscriberEvent_connected},
        {eCAL::eSubscriberEvent::disconnected, eCAL_eSubscriberEvent_disconnected},
        {eCAL::eSubscriberEvent::dropped, eCAL_eSubscriberEvent_dropped}
    };

    sub_event_callback_data_c_->event_type = subscriber_event_map.at(sub_event_callback_data_.event_type);
    sub_event_callback_data_c_->event_time = sub_event_callback_data_.event_time;
    Assign_SDataTypeInformation(&sub_event_callback_data_c_->publisher_datatype, sub_event_callback_data_.publisher_datatype);
  }

  void Assign_SReceiveCallbackData(struct eCAL_SReceiveCallbackData* receive_callback_data_c_, const eCAL::SReceiveCallbackData receive_callback_data_)
  {
    receive_callback_data_c_->buffer = receive_callback_data_.buffer;
    receive_callback_data_c_->buffer_size = receive_callback_data_.buffer_size;
    receive_callback_data_c_->send_timestamp = receive_callback_data_.send_timestamp;
    receive_callback_data_c_->send_clock = receive_callback_data_.send_clock;
  }
}

extern "C"
{
  struct eCAL_Subscriber
  {
    eCAL::CSubscriber* handle;
    eCAL_STopicId topic_id;
    eCAL_SDataTypeInformation data_type_info;
  };

  ECALC_API eCAL_Subscriber* eCAL_Subscriber_New(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const struct eCAL_Subscriber_Configuration* subscriber_configuration_)
  {
    assert(topic_name_ != NULL);
    eCAL::SDataTypeInformation data_type_information;
    eCAL::Subscriber::Configuration subscriber_configuration = eCAL::GetSubscriberConfiguration();

    if (data_type_information_ != NULL)
      Assign_SDataTypeInformation(data_type_information, data_type_information_);
    if (subscriber_configuration_ != NULL)
      Assign_Subscriber_Configuration(subscriber_configuration, subscriber_configuration_);

    return new eCAL_Subscriber{ new eCAL::CSubscriber(topic_name_, data_type_information, subscriber_configuration) };
  }

  ECALC_API eCAL_Subscriber* eCAL_Subscriber_New2(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const eCAL_SubEventCallbackT sub_event_callback_, const struct eCAL_Subscriber_Configuration* subscriber_configuration_)
  {
    assert(topic_name_ != NULL);
    eCAL::SDataTypeInformation data_type_information;
    eCAL::Subscriber::Configuration subscriber_configuration = eCAL::GetSubscriberConfiguration();

    if (data_type_information_ != NULL)
      Assign_SDataTypeInformation(data_type_information, data_type_information_);

    const auto sub_event_callback = [sub_event_callback_](const eCAL::STopicId& topic_id_, const eCAL::SSubEventCallbackData& sub_event_callback_data_)
    {
        struct eCAL_STopicId topic_id_c;
        struct eCAL_SSubEventCallbackData sub_event_callback_data_c;

        Assign_STopicId(&topic_id_c, topic_id_);
        Assign_SSubEventCallbackData(&sub_event_callback_data_c, sub_event_callback_data_);
        sub_event_callback_(&topic_id_c, &sub_event_callback_data_c);
    };

    if (subscriber_configuration_ != NULL)
      Assign_Subscriber_Configuration(subscriber_configuration, subscriber_configuration_);

    return new eCAL_Subscriber{ new eCAL::CSubscriber(topic_name_, data_type_information, sub_event_callback_ != NULL ? sub_event_callback : eCAL::SubEventCallbackT(), subscriber_configuration) };
  }

  ECALC_API void eCAL_Subscriber_Delete(eCAL_Subscriber* subscriber_)
  {
    assert(subscriber_ != NULL);
    delete subscriber_->handle;
    delete subscriber_;
  }

  ECALC_API int eCAL_Subscriber_SetReceiveCallback(eCAL_Subscriber* subscriber_, eCAL_ReceiveCallbackT callback_)
  {
    assert(subscriber_ != NULL && callback_ != NULL);
    const auto callback = [callback_](const eCAL::STopicId& publisher_id_, const eCAL::SDataTypeInformation& data_type_information_, const eCAL::SReceiveCallbackData& receive_callback_data_)
    {
      eCAL_STopicId publisher_id_c;
      eCAL_SDataTypeInformation data_type_information_c;
      eCAL_SReceiveCallbackData receive_callback_data_c;

      Assign_STopicId(&publisher_id_c, publisher_id_);
      Assign_SDataTypeInformation(&data_type_information_c, data_type_information_);
      Assign_SReceiveCallbackData(&receive_callback_data_c, receive_callback_data_);
      
      callback_(&publisher_id_c, &data_type_information_c, &receive_callback_data_c);
    };

    return static_cast<int>(!subscriber_->handle->SetReceiveCallback(callback));
  }

  ECALC_API int eCAL_Subscriber_RemoveReceiveCallback(eCAL_Subscriber* subscriber_)
  {
    assert(subscriber_ != NULL);
    return static_cast<int>(!subscriber_->handle->RemoveReceiveCallback());
  }

  ECALC_API size_t eCAL_Subscriber_GetPublisherCount(eCAL_Subscriber* subscriber_)
  {
    assert(subscriber_ != NULL);
    return subscriber_->handle->GetPublisherCount();
  }

  ECALC_API const char* eCAL_Subscriber_GetTopicName(eCAL_Subscriber* subscriber_)
  {
    assert(subscriber_ != NULL);
    return subscriber_->handle->GetTopicName().c_str();
  }

  ECALC_API const struct eCAL_STopicId* eCAL_Subscriber_GetTopicId(eCAL_Subscriber* subscriber_)
  {
    assert(subscriber_ != NULL);
    Assign_STopicId(&subscriber_->topic_id, subscriber_->handle->GetTopicId());
    return &subscriber_->topic_id;
  }

  ECALC_API const struct eCAL_SDataTypeInformation* eCAL_Subscriber_GetDataTypeInformation(eCAL_Subscriber* subscriber_)
  {
    assert(subscriber_ != NULL);
    Assign_SDataTypeInformation(&subscriber_->data_type_info, subscriber_->handle->GetDataTypeInformation());
    return &subscriber_->data_type_info;
  }
}
#endif // ECAL_CORE_SUBSCRIBER
