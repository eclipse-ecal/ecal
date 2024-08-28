/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  eCAL registration provider
 *
 * All process internal publisher/subscriber, server/clients register here with all their attributes.
 *
 * These information will be send cyclic (registration refresh) via UDP to external eCAL processes.
 *
**/

#pragma once

#include "serialization/ecal_struct_sample_registration.h"


namespace eCAL
{
  class CRegistrationSender
  {
  public:
    CRegistrationSender() = default;
    virtual ~CRegistrationSender() = default;

    // Copy constructor
    CRegistrationSender(const CRegistrationSender& other) = delete;

    // Copy assignment operator
    CRegistrationSender& operator=(const CRegistrationSender & other) = delete;

    // Move constructor
    CRegistrationSender(CRegistrationSender && other) noexcept = delete;

    // Move assignment operator
    CRegistrationSender& operator=(CRegistrationSender && other) noexcept = delete;

    //virtual bool SendSample(const Registration::Sample& sample_) = 0;
    virtual bool SendSampleList(const Registration::SampleList& sample_list) = 0;
  };
}
