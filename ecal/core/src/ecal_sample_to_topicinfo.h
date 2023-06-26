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
 * @brief  helper class to copy from eCAL::pb:Sample to STopicInformation
**/

#pragma once

#include <ecal/core/pb/ecal.pb.h>
#include <ecal/types/topic_information.h>

#include <ecal/ecal_util.h>

namespace eCAL
{

  inline STopicInformation eCALSampleToTopicInformation(const eCAL::pb::Sample& sample)
  {
    STopicInformation topic;
    const auto& tinfo = sample.topic().tinfo();
    topic.encoding = tinfo.encoding();
    topic.type = tinfo.type();
    topic.descriptor = tinfo.desc();
    return topic;
  }
  
  // This function can be removed in eCAL6. For the time being we need to enrich incoming samples with additional topic information.
  inline void ModifyIncomingSampleForBackwardsCompatibility(const eCAL::pb::Sample& sample, eCAL::pb::Sample& modified_sample)
  {
    modified_sample.CopyFrom(sample);
    if (modified_sample.has_topic() && !modified_sample.topic().has_tinfo())
    {
      auto* topic_info = modified_sample.mutable_topic()->mutable_tinfo();
      auto split_type = Util::SplitCombinedTopicType(modified_sample.topic().ttype());
      topic_info->set_encoding(split_type.first);
      topic_info->set_type(split_type.second);
      topic_info->set_desc(modified_sample.topic().tdesc());
    }
  }



}