/* ========================= eCAL LICENSE =================================
 *
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

#include "ecal_reader_manager.h"

using namespace eCAL;

SubscriptionHandle eCAL::CReaderManager::AddSubscription(const SSubscriptionParameters& parameters_, ReceiveCallbackT data_callback, ConnectionChangeCallback connection_changed_callback)
{
  return SubscriptionHandle();
  // We need to cache the subscription parameters and the callbacks.
  // When a publisher registration comes in, we need to check if any subscription matches and then create the necessary layer readers.
  // We also need to track which subscriber communicates on which layer
  // 
  // We then need an efficient way to filter the publisher registrations
  // If nothing changed, there is no need to apply anything
}

void eCAL::CReaderManager::RemoveSubscription(SubscriptionHandle handle_)
{
  // Let's see what we have to do here.
  // we need to remove the subscription to all applied layers.

  // 
}


void CReaderManager::ApplyPublisherRegistration(const Registration::Sample& ecal_sample_)
{
  // Later on, we probably want to decouple this, e.g. push registrations int queue and process the queue

  // First thing we need to do is to check if we have any subscriber for the topic of the registration
  // should we cache incoming samples?
  // This might allow us to establish connections faster when a subscription comes in after the registration
  // However, so let's not do it.
  
  /*
  for (auto subscription : getSubscriptions(ecal_sample_))
  {
    if (IsNewConnection(subscription, sample))
    {

    }

    // Did anything change in the connection (e.g. new memfile names?)
    else if (IsUpdated(subscription, ecal_sample))
    {

    }
    */
  }





  /*
  const auto& ecal_topic = ecal_sample_.topic;
  const std::string& topic_name = ecal_topic.topic_name;

  // check topic name
  if (topic_name.empty()) return;

  const auto& publication_info = ecal_sample_.identifier;
  const SDataTypeInformation& topic_information = ecal_topic.datatype_information;

  CSubscriberImpl::SLayerStates layer_states;
  for (const auto& layer : ecal_topic.transport_layer)
  {
    // transport layer versions 0 and 1 did not support dynamic layer enable feature
    // so we set assume layer is enabled if we receive a registration in this case
    if (layer.enabled || layer.version < 2)
    {
      switch (layer.type)
      {
      case tl_ecal_udp:
        layer_states.udp.write_enabled = true;
        break;
      case tl_ecal_shm:
        layer_states.shm.write_enabled = true;
        break;
      case tl_ecal_tcp:
        layer_states.tcp.write_enabled = true;
        break;
      default:
        break;
      }
    }
  }

  // register publisher
  const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
  auto res = m_topic_name_subscriber_map.equal_range(topic_name);
  for (auto iter = res.first; iter != res.second; ++iter)
  {
    // apply layer specific parameter
    for (const auto& transport_layer : ecal_sample_.topic.transport_layer)
    {
      iter->second->ApplyLayerParameter(publication_info, transport_layer.type, transport_layer.par_layer);
    }
    iter->second->ApplyPublisherRegistration(publication_info, topic_information, layer_states);
  }
}
  */

void CReaderManager::ApplyPublisherUnregistration(const Registration::Sample& ecal_sample_)
{
  /*
  const auto& ecal_topic = ecal_sample_.topic;
  const std::string& topic_name = ecal_topic.topic_name;

  // check topic name
  if (topic_name.empty()) return;

  const auto& publication_info = ecal_sample_.identifier;
  const SDataTypeInformation& topic_information = ecal_topic.datatype_information;

  // unregister publisher
  const std::shared_lock<std::shared_timed_mutex> lock(m_topic_name_subscriber_mutex);
  auto res = m_topic_name_subscriber_map.equal_range(topic_name);
  for (auto iter = res.first; iter != res.second; ++iter)
  {
    iter->second->ApplyPublisherUnregistration(publication_info, topic_information);
  }
  */
}

/*
// We need this in a lambda ->

bool CSubscriberImpl::ShouldApplySampleBasedOnClock(const SPublicationInfo& publication_info_, long long clock_) const
{
  // If counter is already present (duplicate), or unsure if it was present, the sample is not applied
  if (m_publisher_message_counter_map.HasCounter(publication_info_, clock_) != CounterCacheMapT::CounterInCache::False)
  {
#ifndef NDEBUG
    // log it
    Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::AddSample discard sample because of multiple receive");
#endif

    return false;
  }

  // The sample counter is strictly monotonically increasing. If not so, we received an old message.
  // If it is applied or not depends on the configuration. Anyways, a message at low debug level is logged.
  if (!m_publisher_message_counter_map.IsMonotonic(publication_info_, clock_))
  {
#ifndef NDEBUG
    std::string msg = "Subscriber: \'";
    msg += m_attributes.topic_name;
    msg += "\'";
    msg += " received a message in the wrong order";
    Logging::Log(Logging::log_level_warning, msg);
#endif

    // @TODO: We should not have a global config call here. This should be an attribute of the subscriber!
    if (Config::GetDropOutOfOrderMessages())
    {
      return false;
    }
    else
    {
      return true;
    }
  }

  return true;
}

  private:
    using CounterCacheMapT = CounterCacheMap<SPublicationInfo>;
    CounterCacheMapT                          m_publisher_message_counter_map;
    */