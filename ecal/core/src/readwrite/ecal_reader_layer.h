/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include <ecal/ecal_qos.h>

#include <memory>
#include <string>

namespace eCAL
{
  // ecal data layer specific parameters
  // will be transmitted between writer and reader
  struct SReaderLayerPar
  {
    std::string host_name;
    std::string process_id;
    std::string topic_name;
    std::string topic_id;
    std::string parameter;
  };

  // ecal data layer base class
  template <typename T>
  class CReaderLayer
  {
  public:
    CReaderLayer() = default;

    virtual ~CReaderLayer() = default;

    virtual void Initialize() = 0;

    virtual void AddSubscription(const std::string& host_name_, const std::string& topic_name_, const std::string& topic_id_, QOS::SReaderQOS qos_) = 0;
    virtual void RemSubscription(const std::string& host_name_, const std::string& topic_name_, const std::string& topic_id_) = 0;

    virtual void SetConnectionParameter(SReaderLayerPar& par_) = 0;

    static std::shared_ptr<T> Get()
    {
      if (!layer)
      {
        layer = std::make_shared<T>();
      }
      return layer;
    }

  private:
    static std::shared_ptr<T> layer;
  };
};
