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

#include "registration/udp/ecal_registration_receiver_udp.h"

#include "io/udp/ecal_udp_sample_receiver.h"
#include "io/udp/ecal_udp_configurations.h"
#include "serialization/ecal_serialize_sample_registration.h"
#include <ecal/config.h>

#include "registration/udp/config/builder/udp_attribute_builder.h"

using namespace eCAL;

eCAL::CRegistrationReceiverUDP::CRegistrationReceiverUDP(RegistrationApplySampleCallbackT apply_sample_callback, const Registration::UDP::SReceiverAttributes& attr_)
  : m_registration_receiver(std::make_unique<UDP::CSampleReceiver>(
    Registration::UDP::ConvertToIOUDPReceiverAttributes(attr_),
    [](const std::string& /*sample_name_*/) {return true; },
    [apply_sample_callback](const char* serialized_sample_data_, size_t serialized_sample_size_) {
      Registration::Sample sample;
      if (!DeserializeFromBuffer(serialized_sample_data_, serialized_sample_size_, sample)) return false;
      return apply_sample_callback(sample);
    }
    ))
{}

eCAL::CRegistrationReceiverUDP::~CRegistrationReceiverUDP() = default;
