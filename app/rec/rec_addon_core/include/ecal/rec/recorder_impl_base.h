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

#include <ecal/rec/recorder_impl_base_types.h>
#include <ecal/rec/concurrent_status_interface.h>

#include <string>
#include <memory>
#include <functional>
#include <cstdint>


namespace eCAL
{
  namespace rec
  {
    namespace addon
    {
      class RecorderImplBase : public ConcurrentStatusInterface<std::string>
      {
      public:
        virtual ~RecorderImplBase() {}

        virtual bool Initialize() = 0;
        virtual bool Deinitialize() = 0;
        virtual bool StartRecording(std::int64_t measurement_id, const std::string& path) = 0;
        virtual bool FlushFrame(std::int64_t measurement_id, const std::shared_ptr<BaseFrame>& frame) = 0;
        virtual bool StopRecording(std::int64_t measurement_id) = 0;
        virtual Info GetInfo() const = 0;

        void SetRecordFrameCallback(const std::function<bool(const std::shared_ptr<BaseFrame>&)>& callback)
        {
          RecordFrame = callback;
        }

        void RemoveRecordFrameCallback()
        {
          RecordFrame = [](const std::shared_ptr<BaseFrame>&) -> bool { return false; };
        }
      protected:
        std::function<bool(const std::shared_ptr<BaseFrame>&)> RecordFrame = [](const std::shared_ptr<BaseFrame>&) -> bool { return false; };
      };

      extern std::unique_ptr<RecorderImplBase> recorder_impl;

    }
  }
}
