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

#include "shm_attribute_builder.h"

namespace eCAL
{
  namespace eCALReader
  {
    SHM::SAttributes BuildSHMAttributes(const eCALReader::SAttributes& attr_)
    {
      SHM::SAttributes attributes;

      attributes.process_id              = attr_.process_id;
      attributes.registration_timeout_ms = attr_.registration_timeout_ms;
      
      return attributes;
    }
  }

  namespace eCALWriter
  {
    SHM::SAttributes BuildSHMAttributes(const eCALWriter::SAttributes& attr_)
    {
      SHM::SAttributes attributes;

      attributes.acknowledge_timeout_ms  = attr_.shm.acknowledge_timeout_ms;
      attributes.memfile_buffer_count    = attr_.shm.memfile_buffer_count;
      attributes.memfile_reserve_percent = attr_.shm.memfile_reserve_percent;
      attributes.memfile_min_size_bytes  = attr_.shm.memfile_min_size_bytes;

      attributes.topic_name = attr_.topic_name;
      attributes.host_name  = attr_.host_name;
      
      return attributes;
    }
  }
}