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

#include <memory>
#include <string>

namespace eCAL
{
  // ecal data layer specific parameters
  // transmitted from a writer to a reader
  struct SReaderLayerPar
  {
    std::string                 host_name;
    int32_t                     process_id = 0;
    std::string                 topic_name;
    EntityIdT     topic_id = 0;
    Registration::ConnectionPar parameter;
  };

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

    static std::shared_ptr<T> Get()
    {
      static std::shared_ptr<T> layer = nullptr;
      if (!layer)
      {
        layer = std::make_shared<T>();
      }
      return layer;
    }
  };
}
