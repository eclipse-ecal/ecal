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
#include <ecal/ecal_types.h>

#include <ecal/ecal_util.h>

namespace eCAL
{

  STopicInformation eCALSampleToTopicInformation(const eCAL::pb::Sample& sample)
  {
    STopicInformation topic;

    if (sample.topic().tinfo().IsInitialized())
    {
      const auto& tinfo = sample.topic().tinfo();
      topic.encoding = tinfo.encoding();
      topic.type = tinfo.type();
      topic.descriptor = tinfo.desc();
    }
    // remove else part in eCAL6, compatibility with eCAL < 5.12.0
    else 
    {
      auto split_type = Util::SplitCombinedTopicType(sample.topic().ttype());
      topic.encoding = split_type.first;
      topic.type = split_type.second;
      topic.descriptor = sample.topic().tdesc();
    }

    return topic;
  }
  




}