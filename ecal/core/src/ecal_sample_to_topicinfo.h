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

#include <ecal/core/pb/ecal.pb.h>
#include <ecal/ecal_types.h>

#include <ecal/ecal_util.h>

namespace eCAL
{

  inline SDataTypeInformation eCALSampleToTopicInformation(const eCAL::pb::Sample& sample)
  {
    SDataTypeInformation topic;
    const auto& tdatatype = sample.topic().tdatatype();
    topic.encoding = tdatatype.encoding();
    topic.name = tdatatype.name();
    topic.descriptor = tdatatype.desc();
    return topic;
  }
  
  // This function can be removed in eCAL6. For the time being we need to enrich incoming samples with additional topic information.
  inline void ModifyIncomingSampleForBackwardsCompatibility(const eCAL::pb::Sample& sample, eCAL::pb::Sample& modified_sample)
  {
    modified_sample.CopyFrom(sample);
    if (modified_sample.has_topic() && !modified_sample.topic().has_tdatatype())
    {
      auto* topic_datatype = modified_sample.mutable_topic()->mutable_tdatatype();
      auto split_type = Util::SplitCombinedTopicType(modified_sample.topic().ttype());
      topic_datatype->set_name(split_type.second);
      topic_datatype->set_encoding(split_type.first);
      topic_datatype->set_desc(modified_sample.topic().tdesc());
    }
  }



}