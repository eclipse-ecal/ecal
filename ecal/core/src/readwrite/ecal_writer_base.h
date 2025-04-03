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
 * @brief  data writer base class
**/

#pragma once

#include <ecal/pubsub/payload_writer.h>

#include "ecal_writer_data.h"
#include "ecal_writer_info.h"

#include "serialization/ecal_struct_sample_registration.h"

#include <atomic>
#include <string>

namespace eCAL
{
  class CDataWriterBase
  {
  public:
    virtual ~CDataWriterBase() = default;

    virtual SWriterInfo GetInfo() = 0;

    virtual void ApplySubscription(const std::string& /*host_name_*/, const int32_t /*process_id_*/, const EntityIdT& /*topic_id_*/, const std::string& /*conn_par_*/) {};
    virtual void RemoveSubscription(const std::string& /*host_name_*/, const int32_t /*process_id_*/, const EntityIdT& /*topic_id_*/) {};

    virtual Registration::ConnectionPar GetConnectionParameter() { return {}; };

    virtual bool PrepareWrite(const SWriterAttr& /*attr_*/) { return false; };
    virtual bool Write(CPayloadWriter& /*payload_*/, const SWriterAttr& /*attr_*/) { return false; };
    virtual bool Write(const void* /*buf_*/, const SWriterAttr& /*attr_*/) { return false; };
  };
}
