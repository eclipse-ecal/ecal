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
 * @brief  eCAL registration sender UDP
 *
 * A device which sends out registration information via UDP
 *
**/

#pragma once

#include "registration/ecal_registration_sender.h"

#include "io/udp/ecal_udp_sample_sender.h"

namespace eCAL
{
  class CRegistrationSenderUDP : public CRegistrationSender
  {
  public:
    CRegistrationSenderUDP();
    ~CRegistrationSenderUDP() override;

    // Special member functionss
    CRegistrationSenderUDP(const CRegistrationSenderUDP& other) = delete;
    CRegistrationSenderUDP& operator=(const CRegistrationSenderUDP& other) = delete;
    CRegistrationSenderUDP(CRegistrationSenderUDP&& other) noexcept = delete;
    CRegistrationSenderUDP& operator=(CRegistrationSenderUDP&& other) noexcept = delete;

    bool SendSampleList(const Registration::SampleList& sample_list) override;

  private:
    bool SendSample(const Registration::Sample& sample_);

    UDP::CSampleSender m_reg_sample_snd;
  };
}