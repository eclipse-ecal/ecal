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

#pragma once
#include <functional>

#include <readwrite/ecal_reader_layer.h>
#include <serialization/ecal_struct_sample_registration.h>

#include <ecal/types.h>

namespace eCAL
{
  using LayerPriority = int32_t;

  struct SSubscriptionParameters
  {
    std::string                 host_name;
    int32_t                     process_id = 0;
    std::string                 topic_name;
    EntityIdT                   topic_id = 0;

    ActiveLayers                layers;
  };

  enum class ConnectionState
  {
    PENDING,       // Subscriber has subscribed, but we haven't yet seen a matching registration sample
    ESTABLISHING,  // We have received 
    ESTABLISHED,   // A connection has been established on a negotiated transport layer
    DISCONNECTED    // The subscriber has disconnected
  };

  // We need to track each connection per publisher.
  // E.g. a subscriber can receive data from multiple publishers
  // We need to track that information for each subscriber, e.g. a list / vector / map

  class CReaderManager
  {
  public:
    CReaderManager() {}

    // A subscriber calls this function to request to receive data / unsubscribe
    SubscriptionHandle AddSubscription(const SSubscriptionParameters& parameters_, ReceiveCallbackT data_callback, ConnectionChangeCallback connection_changed_callback);
    void RemoveSubscription(SubscriptionHandle handle_);

    // The global registration receiver calls this function to notify about new / removed publications
    // The reader manager can go ahead and apply this information to actually establish connections.
    void ApplyPublisherRegistration(const Registration::Sample& sample);
    void ApplyPublisherUnregistration(const Registration::Sample& unregistration_sample);

  private:
    // We need to track connection states of each subscription.


    std::unordered_map<SubscriptionHandle, ConnectionState> m_connection_states;

  };
}



