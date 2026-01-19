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
 * @brief  shared memory layer
**/

#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "readwrite/ecal_reader_layer.h"
#include "config/attributes/reader_shm_attributes.h"

namespace eCAL
{
  ////////////////
  // LAYER
  ////////////////
  class CSHMReaderLayer : CTransportLayerInstance
  {
  public:
    CSHMReaderLayer(const eCAL::eCALReader::SHM::SAttributes& attr_);
    ~CSHMReaderLayer() override = default;

    bool AcceptsConnection(const PublisherConnectionParameters& publisher, const SubscriberConnectionParameters& subscriber) const override;
    CTransportLayerInstance::ConnectionToken AddConnection(const PublisherConnectionParameters& publisher, const ReceiveCallbackT& on_data, const ConnectionChangeCallback& on_connection_changed) override;
    // How about updating the connection (SHM memfile list changed?)
    // Maybe via connection token?
    // void RemoveConnection(ConnectionToken connection_handle_) override;

  private:
    eCAL::eCALReader::SHM::SAttributes m_attributes;
  };
}
