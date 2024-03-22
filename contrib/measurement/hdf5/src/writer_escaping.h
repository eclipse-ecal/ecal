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
 * @file   writer.h
 * @brief  Base class for low level measurement writing operations
**/

#pragma once

#include <functional>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <optional>

#include <ecal/measurement/base/writer.h>

namespace eCAL
{
  namespace experimental
  {
    namespace measurement
    {
      namespace base
      {
        /**
         * @brief eCAL Measurement Writer API
        **/
        class EscapingWriter : public Writer
        {
        public:
          EscapingWriter(WriterCreator writer_creator_, const FileOptions& options_);

          void SetChannelDataTypeInformation(const eCAL::experimental::measurement::base::Channel& channel_name, const DataTypeInformation& info) override;

          void AddEntryToFile(const eCAL::experimental::measurement::base::Channel& channel, const WriterEntry& entry) override;

          static WriterCreator GetCreator(WriterCreator creator_);
        private:
          std::unique_ptr<Writer> writer;
        };
      }
    }
  }
}
