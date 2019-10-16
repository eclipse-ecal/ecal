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

#include "abstract_recorder.h"

#include <memory>

namespace eCAL
{
  namespace rec
  {
    class EcalRec;

    class LocalRecorder : public AbstractRecorder
    {
    public:
      explicit LocalRecorder(const std::string& hostname, const std::shared_ptr<eCAL::rec::EcalRec>& ecal_rec_instance, const RecorderSettings& initial_settings, bool initially_connected_to_ecal);
      ~LocalRecorder();

    public:
      virtual void SetClientConnectionEnabled(bool connect) override;
      virtual bool IsClientConnectionEnabled() const override;

      virtual void SetSettings(const RecorderSettings& settings) override;
      virtual void SetCommand(const RecorderCommand& command) override;

      virtual void InitiateConnectionShutdown(const RecorderCommand& last_command) override;

      virtual bool IsAlive() const override;
      virtual RecorderState GetState() const override;

      virtual bool IsRequestPending() const override;
      virtual void WaitForPendingRequests() const override;

      virtual std::pair<bool, std::string> GetLastResponse() const override;

    private:
      std::shared_ptr<eCAL::rec::EcalRec> ecal_rec_instance_;

      bool             connection_activated_;
      bool             is_in_sync_;

      std::pair<bool, std::string> last_response_;

      RecorderSettings complete_settings_;
      bool             should_be_connected_to_ecal_;
    };
  }
}