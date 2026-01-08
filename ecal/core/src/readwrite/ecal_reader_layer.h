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
 * @brief  data reader layer base class
**/

#pragma once

#include "serialization/ecal_struct_sample_registration.h"
#include "serialization/ecal_struct_sample_payload.h"

#include <cassert>
#include <memory>
#include <string>
#include <functional>

#include <ecal/types.h>
#include <ecal/pubsub/types.h>

namespace eCAL
{
  enum class LayerType
  {
    UDP,
    SHM,
    TCP,
    COUNT
  };

  class ActiveLayers {
  public:
    using Storage = std::uint32_t; // plenty for < 10 layers

    constexpr ActiveLayers() = default;
    constexpr explicit ActiveLayers(Storage rawMask) : mask_(rawMask) {}

    // --- queries ---
    [[nodiscard]] constexpr bool has(LayerType layer) const noexcept {
      return (mask_ & bit(layer)) != 0;
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return mask_ == 0; }
    [[nodiscard]] constexpr Storage raw() const noexcept { return mask_; }

    // --- modifiers (chainable) ---
    constexpr ActiveLayers& set(LayerType layer) noexcept {
      mask_ |= bit(layer);
      return *this;
    }

    constexpr ActiveLayers& clear(LayerType layer) noexcept {
      mask_ &= ~bit(layer);
      return *this;
    }

    constexpr ActiveLayers& toggle(LayerType layer) noexcept {
      mask_ ^= bit(layer);
      return *this;
    }

    // Convenient: set/clear based on a boolean
    constexpr ActiveLayers& set(LayerType layer, bool enabled) noexcept {
      return enabled ? set(layer) : clear(layer);
    }

    // Bulk ops
    constexpr ActiveLayers& clear_all() noexcept {
      mask_ = 0;
      return *this;
    }

    constexpr ActiveLayers& set_all() noexcept {
      mask_ = all_bits();
      return *this;
    }

    // Combine / subtract sets (also useful for chaining)
    constexpr ActiveLayers& add(ActiveLayers other) noexcept {
      mask_ |= other.mask_;
      return *this;
    }

    constexpr ActiveLayers& remove(ActiveLayers other) noexcept {
      mask_ &= ~other.mask_;
      return *this;
    }

    // --- non-member-friendly operators ---
    friend constexpr ActiveLayers operator|(ActiveLayers a, ActiveLayers b) noexcept {
      return ActiveLayers{ Storage(a.mask_ | b.mask_) };
    }
    friend constexpr ActiveLayers operator&(ActiveLayers a, ActiveLayers b) noexcept {
      return ActiveLayers{ Storage(a.mask_ & b.mask_) };
    }
    friend constexpr bool operator==(ActiveLayers a, ActiveLayers b) noexcept {
      return a.mask_ == b.mask_;
    }

    // Helper to build a set from one layer
    static constexpr ActiveLayers one(LayerType layer) noexcept {
      ActiveLayers s;
      s.set(layer);
      return s;
    }

  private:
    Storage mask_{ 0 };

    static constexpr Storage bit(LayerType layer) noexcept {
      const auto idx = static_cast<std::underlying_type_t<LayerType>>(layer);
      return Storage{ 1 } << idx;
    }

    static constexpr Storage all_bits() noexcept {
      // Count is the number of layers
      const auto n = static_cast<std::underlying_type_t<LayerType>>(LayerType::COUNT);
      return (n == 0) ? 0 : (Storage{ 1 } << n) - 1;
    }

    static_assert(static_cast<unsigned>(LayerType::COUNT) <= (sizeof(Storage) * 8),
      "ActiveLayers::Storage too small for LayerType::COUNT");
  };

  // TODO: GetSHMTransportDomain should rather be a layer parameter
  class SubscriberConnectionParameters
  {
    STopicId                    id;
    SDataTypeInformation        datatype_info;
    ActiveLayers                active_layers;
    std::string                 shm_transport_domain;
  public:
    const std::string& GetTopicName() const { return id.topic_name; }
    const std::string& GetSHMTransportDomain() const { return shm_transport_domain; }
    bool IsLayerActive(LayerType layer) const { return active_layers.has(layer); }
    //Registration::TLayer GetLayerParameter(LayerType layer) const {}
  };

  class PublisherConnectionParameters
  {
  public:
    PublisherConnectionParameters(const Registration::Sample& ecal_sample_)
      : publisher_sample(ecal_sample_)
    {
    }

    const std::string& GetTopicName() const { return publisher_sample.topic.topic_name; }
    const SDataTypeInformation& GetDataTypeInformation() const { return publisher_sample.topic.datatype_information; }
    const STopicId GetTopicId() const {
      STopicId topic_id;
      topic_id.topic_name = publisher_sample.topic.topic_name;
      topic_id.topic_id.entity_id = publisher_sample.identifier.entity_id;
      topic_id.topic_id.process_id = publisher_sample.identifier.process_id;
      topic_id.topic_id.host_name = publisher_sample.identifier.host_name;
      return topic_id;
    }
    // TODO: GetHostName() and GetSHMTransportDomain() should rather be layer parameters
    const std::string& GetHostName() const { return publisher_sample.identifier.host_name; }
    const std::string& GetSHMTransportDomain() const { return publisher_sample.topic.shm_transport_domain; }
    bool IsLayerActive(LayerType /*layer*/) const { return true; }
    Registration::TLayer GetLayerParameter(LayerType /*layer*/) const { return {}; }


  private:
    const Registration::Sample& publisher_sample;
  };

  struct SubscriberConnectionInstance
  {
    STopicId                    publisher_id;
    SDataTypeInformation        publisher_datatype_info;
    // It would make a bit more sense to have one layer only, but maybe later, this way we can use it for updates and tracking
    ActiveLayers                publisher_connected_layers;
  };

  enum class SubscriberConnectionChange
  {
    NewConnection,
    RemovedConnection
  };
  using ConnectionChangeCallback = std::function<void(SubscriberConnectionChange, const SubscriberConnectionInstance&)>;

  // Class to hold subscriber connection information
  class SubscriberConnectionInfo
  {
  public:
    SubscriberConnectionInfo() = default;

    void AddConnection(const SubscriberConnectionInstance& connection_info_)
    {
      const auto entitiy_id = connection_info_.publisher_id.topic_id.entity_id;
      auto connection = connections.find(entitiy_id);
      if (connection == connections.end())
      {
        connections[entitiy_id] = connection_info_;
      }
      else
      {
        connection->second.publisher_connected_layers.add(connection_info_.publisher_connected_layers);
      }
    }

    void RemoveConnection(const SubscriberConnectionInstance& connection_info_)
    {
      const auto entitiy_id = connection_info_.publisher_id.topic_id.entity_id;
      auto connection = connections.find(entitiy_id);
      assert(connection != connections.end());
      auto& connected_layers_in_map = connection->second.publisher_connected_layers;
      connected_layers_in_map.remove(connection_info_.publisher_connected_layers);
      if (connected_layers_in_map.empty())
      {
        connections.erase(connection);
      }
    }

    bool IsConnected() const
    {
      return !connections.empty();
    }

    size_t ConnectionCount() const
    {
      return connections.size();
    }

  private:
    std::map<EntityIdT, SubscriberConnectionInstance> connections;
  };

  // This class manages all connections for a specific layer type
  class CTransportLayerInstance
  {
  public:
    // When the token is destroyed, the connection is removed automatically
    // and destructors that will clean up resources
    class ConnectionToken
    {
    public:
      ConnectionToken() = default;

      // Move-only
      ConnectionToken(ConnectionToken&& other) noexcept
        : disconnect_(std::move(other.disconnect_))
        , update_(std::move(other.update_))
      {
        other.disconnect_ = {};
        other.update_ = {};
      }

      ConnectionToken& operator=(ConnectionToken&& other) = delete;
      ConnectionToken(const ConnectionToken&) = delete;
      ConnectionToken& operator=(const ConnectionToken&) = delete;

      ~ConnectionToken()
      {
        if (disconnect_)
        {
          try { disconnect_(); }
          catch (...) {}
        }
      }

      // Infrequent updates
      void UpdateConnection(const PublisherConnectionParameters& publisher)
      {
        if (update_) update_(publisher);
      }

      // Factories
      template <typename DisconnectFn, typename UpdateFn>
      static ConnectionToken Make(DisconnectFn&& disconnect, UpdateFn&& update)
      {
        ConnectionToken t;
        t.disconnect_ = std::forward<DisconnectFn>(disconnect);
        t.update_ = std::forward<UpdateFn>(update);
        return t;
      }

      template <typename DisconnectFn>
      static ConnectionToken Make(DisconnectFn&& disconnect)
      {
        ConnectionToken t;
        t.disconnect_ = std::forward<DisconnectFn>(disconnect);
        return t;
      }

    private:
      std::function<void()> disconnect_;
      std::function<void(const PublisherConnectionParameters&)> update_;
    };


    CTransportLayerInstance(LayerType layer_type)
      : m_layer_type(layer_type)
    {
    }

    virtual ~CTransportLayerInstance() = default;

    LayerType GetLayer() const {
      return m_layer_type;
    }

    // We need to pass in publisher & subscriber information
    virtual bool AcceptsConnection(const PublisherConnectionParameters& publisher, const SubscriberConnectionParameters& subscriber) const = 0;
    virtual ConnectionToken AddConnection(const PublisherConnectionParameters& publisher, const ReceiveCallbackT& on_data, const ConnectionChangeCallback& on_connection_changed) = 0;
    // How about updating the connection (SHM memfile list changed?)
    // Maybe via connection token?
    //virtual void RemoveConnection(ConnectionToken connection_handle_) = 0;

  protected:
    LayerType m_layer_type;
  };

  inline bool LayerEnabledForPublisherAndSubscriber(LayerType layer, const PublisherConnectionParameters& publisher, const SubscriberConnectionParameters& subscriber)
  {
    return publisher.IsLayerActive(layer) && subscriber.IsLayerActive(layer);
  }
}