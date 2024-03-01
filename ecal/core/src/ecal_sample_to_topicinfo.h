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
 * @brief  helper class to copy from eCAL::pb:Sample to SDataTypeInformation
**/

#pragma once

#include <ecal/ecal_types.h>
#include <ecal/ecal_util.h>

#include "serialization/ecal_serialize_sample_registration.h"

namespace eCAL
{
  inline SDataTypeInformation eCALSampleToTopicInformation(const eCAL::Registration::Sample& sample)
  {
    SDataTypeInformation topic;
    const auto& tdatatype = sample.topic.tdatatype;
    topic.encoding   = tdatatype.encoding;
    topic.name       = tdatatype.name;
    topic.descriptor = tdatatype.desc;
    return topic;
  }
}
