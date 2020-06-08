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

#pragma once

#include <unordered_map>
#include <string>

namespace eCAL
{
  namespace rec
  {
    namespace addon
    {
      struct JobStatus
      {
        enum class State
        {
          NotStarted,
          Recording,
          Flushing,
          Finished
        };

        State state;
        bool healthy;
        std::string description;
        std::size_t frame_count;
        std::size_t queue_count;
      };

      using JobStatuses = std::unordered_map<std::int64_t, JobStatus>;
    }
  }
}