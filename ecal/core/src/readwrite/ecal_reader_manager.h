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
#include <string>
#include <map>

#include <readwrite/ecal_reader_layer.h>
#include <serialization/ecal_struct_sample_registration.h>

#include <ecal/types.h>

namespace eCAL
{
  using LayerPriority = int32_t;

  struct SSubscriptionParameters
  {
    STopicId                    topic;
    ActiveLayers                layers;
    ReceiveCallbackT            data_callback;
    ConnectionChangeCallback    connection_changed_callback;
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

  class SubscriptionHandle
  {
    friend class SubscriptionMap;
  public:
    SubscriptionHandle(EntityIdT id, std::string topic)
      : subscriber_id(id), subscriber_topic_name(std::move(topic)) {
    }

  private:
    EntityIdT subscriber_id;
    std::string subscriber_topic_name;
  };

  using TopicName = std::string;

  class SubscriptionMap
  {
  public:
    using TopicName = std::string;
    using Storage = std::multimap<TopicName, SSubscriptionParameters>;

    class Subscriptions
    {
      using StorageIterator = Storage::const_iterator;

      class Iterator
      {
      public:
        explicit Iterator(StorageIterator cursor) : cursor_(cursor) {}

        const SSubscriptionParameters& operator*() const { return cursor_->second; }
        const SSubscriptionParameters* operator->() const { return &cursor_->second; }

        Iterator& operator++() { ++cursor_; return *this; }

        bool operator==(const Iterator& other) const { return cursor_ == other.cursor_; }
        bool operator!=(const Iterator& other) const { return !(*this == other); }

      private:
        StorageIterator cursor_;
      };

    public:
      Subscriptions(StorageIterator begin, StorageIterator end)
        : begin_(begin), end_(end) {
      }

      Iterator begin() const { return Iterator(begin_); }
      Iterator end()   const { return Iterator(end_); }

      bool empty() const { return begin_ == end_; }

    private:
      StorageIterator begin_;
      StorageIterator end_;
    };

    SubscriptionHandle AddSubscription(const SSubscriptionParameters& parameters_)
    {
      subscriptions_.emplace(parameters_.topic.topic_name, parameters_);
      return SubscriptionHandle{ parameters_.topic.topic_id.entity_id, parameters_.topic.topic_name };
    }

    SSubscriptionParameters RemoveSubscription(const SubscriptionHandle& handle)
    {
      auto it = subscriptions_.equal_range(handle.subscriber_topic_name);
      for (auto map_it = it.first; map_it != it.second; ++map_it)
      {
        if (map_it->second.topic.topic_id.entity_id == handle.subscriber_id) {
          SSubscriptionParameters removed_subscription = map_it->second;
          subscriptions_.erase(map_it); 
          return removed_subscription;
        }
      }

      throw std::logic_error("RemoveSubscription: subscription not found");
    }

    Subscriptions SubscriptionsFor(const TopicName& topic) const
    {
      auto range = subscriptions_.equal_range(topic);
      return Subscriptions(range.first, range.second);
    }

  private:
    Storage subscriptions_;
  };


  class CReaderManager
  {
  public:
    CReaderManager() {}

    // A subscriber calls this function to request to receive data / unsubscribe
    SubscriptionHandle AddSubscription(const SSubscriptionParameters& parameters_);
    void RemoveSubscription(SubscriptionHandle handle_);

    // The global registration receiver calls this function to notify about new / removed publications
    // The reader manager can go ahead and apply this information to actually establish connections.
    void ApplyPublisherRegistration(const Registration::Sample& sample);
    void ApplyPublisherUnregistration(const Registration::Sample& unregistration_sample);

  private:
    // We need to track connection states of each subscription.
    SubscriptionMap subscription_map_;
  };


 

}




