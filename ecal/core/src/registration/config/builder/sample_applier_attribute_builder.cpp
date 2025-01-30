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

#include "sample_applier_attribute_builder.h"

namespace eCAL
{
  namespace Registration
  {
    namespace SampleApplier
    {
      SAttributes BuildSampleApplierAttributes(const Registration::SAttributes& attr_)
      {
        SAttributes sample_applier_attr;
        
        sample_applier_attr.network_enabled      = attr_.network_enabled;
        sample_applier_attr.loopback             = attr_.loopback;
        sample_applier_attr.host_name            = attr_.host_name;
        sample_applier_attr.shm_transport_domain = attr_.shm_transport_domain;
        sample_applier_attr.process_id           = attr_.process_id;
        
        return sample_applier_attr;
      }
    }
  }
}
