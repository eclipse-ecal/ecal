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

/**
 * @brief eCAL registration timeout provider
 *
 * Class that tracks incoming samples. 
 * It will call an unregistration sample callback, whenenver a sample has "timed out".
 * This can be treated the same way if the other process had sent an unregister sample.
 *
**/

#pragma once

#include <registration/ecal_registration_types.h>
#include <util/ecal_expmap.h>

#include <mutex>

namespace eCAL
{
  namespace Registration
  {
    bool IsUnregistrationSample(const Registration::Sample& sample_);

    // This function turns a registration sample into an unregistration sample
    // This could happen also in another class / namespace
    Registration::Sample CreateUnregisterSample(const Registration::Sample& sample_);

    // Returns the corresponding unregistration type
    // RegSubscriber -> UnregSubscriber, ...
    // Anything else will return bct_none
    eCmdType GetUnregistrationType(const Registration::Sample& sample_);

    bool IsProcessRegistration(const Registration::Sample& sample_);
    bool IsTopicRegistration(const Registration::Sample& sample_);


    template < class ClockType = std::chrono::steady_clock>
    class CTimeoutProvider
    {
    public:
      CTimeoutProvider(const typename ClockType::duration& timeout_, const RegistrationApplySampleCallbackT& apply_sample_callback_)
      : sample_tracker(timeout_)
      , apply_sample_callback(apply_sample_callback_)
      {}

      bool ApplySample(const Registration::Sample& sample_) {
        // Is unregistration sample?
        if (IsUnregistrationSample(sample_))
        {
          DeleteUnregisterSample(sample_);
        }
        else
        {
          UpdateSample(sample_);
        }
        return true;
      }

      // This function checks for timeouts. This means it scans the map for expired samples
      // It then applies unregistration samples for all internally expired samples.
      void CheckForTimeouts()
      {
        std::map<Registration::SampleIdentifier, Registration::Sample> expired_samples;

        {
          std::lock_guard<std::mutex> lock(sample_tracker_mutex);
          expired_samples = sample_tracker.erase_expired();
        }

        for (const auto& registration_sample : expired_samples)
        {
          Sample unregistration_sample = CreateUnregisterSample(registration_sample.second);
          apply_sample_callback(unregistration_sample);
        }
      }

    private:
      void DeleteUnregisterSample(const Sample& sample_)
      {
        std::lock_guard<std::mutex> lock(sample_tracker_mutex);
        sample_tracker.erase(sample_.identifier);
      }

      void UpdateSample(const Sample& sample_)
      {
        std::lock_guard<std::mutex> lock(sample_tracker_mutex);
        sample_tracker[sample_.identifier] = sample_;
      }

      using SampleTrackerMap = Util::CExpirationMap<Registration::SampleIdentifier, Registration::Sample, ClockType>;
      SampleTrackerMap                 sample_tracker;
      std::mutex                       sample_tracker_mutex;

      RegistrationApplySampleCallbackT apply_sample_callback;
    };
  }
}