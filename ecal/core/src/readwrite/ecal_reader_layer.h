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
  // ecal data layer specific parameters
  // transmitted from a writer to a reader
  using SPublicationInfo = Registration::SampleIdentifier;

  struct SReaderLayerPar
  {
    std::string                 host_name;
    int32_t                     process_id = 0;
    std::string                 topic_name;
    EntityIdT                   topic_id = 0;
    Registration::ConnectionPar parameter;
  };

  enum class SubscriptionHandle
    : EntityIdT
  {
  };

  enum class ConnectionHandle
    : EntityIdT
  {
  };

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

  using SubscriberDataCallback = std::function<void(const eCAL::Payload::Sample& sample_, const void* /*user_data_*/)>;

  // SSubscriptionParameters = SReaderLayerPar;

  // ecal data layer base class
  template <typename T, typename U>
  class CReaderLayer
  {
  public:
    CReaderLayer() = default;

    virtual ~CReaderLayer() = default;

    // initialize layer
    // will be called one time on eCAL initialization
    virtual void Initialize(const U& attr_) = 0;

    // activate / create a specific subscription
    virtual void AddSubscription(const std::string& host_name_, const std::string& topic_name_, const EntityIdT& topic_id_) = 0;

    // deactivate / destroy a specific subscription
    virtual void RemSubscription(const std::string& host_name_, const std::string& topic_name_, const EntityIdT& topic_id_) = 0;

    // connection parameter from writer side
    virtual void SetConnectionParameter(SReaderLayerPar& par_) = 0;
  };

  class CLayerConnectionManager
  {
    CLayerConnectionManager();
    virtual ~CLayerConnectionManager() = default;
    
    virtual LayerType GetLayer() = 0;
    // We need to pass in publisher & subscriber information
    virtual void AcceptsConnection() = 0;
    virtual ConnectionHandle AddConnection() = 0;
    virtual void RemoveConnection(ConnectionHandle connection_handle_) = 0;
  };


}
