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

#pragma once

#include "readwrite/udp/config/attributes/reader_udp_attributes.h"
#include "readwrite/udp/config/attributes/writer_udp_attributes.h"
#include "io/udp/ecal_udp_receiver_attr.h"
#include "io/udp/ecal_udp_sender_attr.h"

namespace eCAL
{
  namespace eCALReader
  {
    namespace UDP
    {
      eCAL::UDP::SReceiverAttr ConvertToIOUDPReceiverAttributes(const SAttributes& attr_);
    }
  }

  namespace eCALWriter
  {
    namespace UDP
    {
      eCAL::UDP::SSenderAttr ConvertToIOUDPSenderAttributes(const SAttributes& attr_);
    }
  }
}