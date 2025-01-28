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
 * @brief  eCAL Sample Applier
 *
 * This class applies incoming samples to everyone who is interested.
**/

#pragma once

#include <cstddef>
#include <ecal/ecal.h>

#include "serialization/ecal_struct_sample_registration.h"
#include "config/attributes/sample_applier_attributes.h"

#include <functional>
#include <map>
#include <mutex>
#include <string>

namespace eCAL
{
  namespace Registration
  {
    class CSampleApplier
    {
    public:
      CSampleApplier(const SampleApplier::SAttributes& attr_);

      bool ApplySample(const Registration::Sample& sample_);

      using ApplySampleCallbackT = std::function<void(const Registration::Sample&)>;
      void SetCustomApplySampleCallback(const std::string& customer_, const ApplySampleCallbackT& callback_);
      void RemCustomApplySampleCallback(const std::string& customer_);

    private:
      bool IsSameProcess(const Registration::Sample& sample_) const;
      bool IsSameHost(const Registration::Sample& sample_) const;
      bool IsSameShmTransportDomain(const Registration::Sample& sample_) const;

      bool IsShmTransportDomainMember(const eCAL::Registration::Sample& sample_) const;

      bool AcceptRegistrationSample(const Registration::Sample& sample_);

      SampleApplier::SAttributes                  m_attributes;

      std::mutex                                  m_callback_custom_apply_sample_map_mtx;
      // We need to check the performance now. Unlike before the pub / subgates also go through the map
      std::map<std::string, ApplySampleCallbackT> m_callback_custom_apply_sample_map;
    };
  }
}
