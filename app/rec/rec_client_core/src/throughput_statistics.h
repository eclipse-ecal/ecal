/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2025 AUMOVIO SE
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

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <rec_client_core/state.h>

namespace eCAL
{
  namespace rec
  {
    class ThroughputStatistics {

    public:
      using duration_t   = std::chrono::milliseconds;
      using time_point_t = std::chrono::time_point<duration_t>;

    public:
      explicit ThroughputStatistics(size_t bins_per_second)
        : bin_width_(std::chrono::duration_cast<duration_t>(std::chrono::seconds(1)) / bins_per_second)
        , bin_count_(bins_per_second + 1)
        , bins_     (bin_count_)
      {}

      // Default the copy and move constructors and assignment operators
      ThroughputStatistics(const ThroughputStatistics&)                 = default;
      ThroughputStatistics& operator=(const ThroughputStatistics&)      = default;
      ThroughputStatistics(ThroughputStatistics&&) noexcept             = default;
      ThroughputStatistics& operator=(ThroughputStatistics&&) noexcept  = default;
      
      // Default destructor
      ~ThroughputStatistics() = default;

      void AddFrame(uint64_t bytes)
      {
        const auto now = time_point_t(std::chrono::duration_cast<duration_t>(std::chrono::steady_clock::now().time_since_epoch()));

        const size_t index = GetBinIndexForTimepoint(now);

        if (bins_[index].start_time + bin_width_ < now)
        {
          RotateBins(now);
        }

        bins_[index].bytes     += bytes;
        bins_[index].frames    += 1;
      }

      Throughput GetThroughput()
      {
        const auto now = time_point_t(std::chrono::duration_cast<duration_t>(std::chrono::steady_clock::now().time_since_epoch()));
        
        // Rotate bins (-> remove old ones)
        RotateBins(now);

        uint64_t total_bytes = 0;
        uint64_t total_frames = 0;

        auto currently_active_bin = GetBinIndexForTimepoint(now);
        for (size_t i = 0; i < bins_.size(); i++)
        {
          if (i == currently_active_bin)
          {
            // Don't sum up the active bin. This makes the statistics lose some
            // responsiveness, but it ensures that the numbers are not
            // flickering as much.
            continue;
          }
          total_bytes += bins_[i].bytes;
          total_frames += bins_[i].frames;
        }

        return Throughput{ total_bytes, total_frames };
      }

    private:
      struct Bin
      {
        time_point_t start_time;
        uint64_t     bytes = 0;
        uint64_t     frames = 0;
      };

      duration_t       bin_width_;
      size_t           bin_count_;
      std::vector<Bin> bins_;

      size_t GetBinIndexForTimepoint(time_point_t time) const
      {
        return (time.time_since_epoch() / bin_width_) % bin_count_;
      }

      time_point_t GetTimePointForBinIndex(size_t index,time_point_t now) const
      {
        const auto current_bin_index      = GetBinIndexForTimepoint(now);
        const auto current_bin_time_point = now - (now.time_since_epoch() % bin_width_);
        const auto index_age              = ((bin_count_ - index + current_bin_index) % bin_count_) * bin_width_;

        return current_bin_time_point - index_age;
      }

      void RotateBins(time_point_t now)
      {
        for (size_t i = 0; i < bins_.size(); i++)
        {
          const auto should_be_timepoint = GetTimePointForBinIndex(i, now);
          if (bins_[i].start_time < should_be_timepoint)
          {
            bins_[i].bytes = 0;
            bins_[i].frames = 0;
            bins_[i].start_time = should_be_timepoint;
          }
        }
      }
    };
  }
}