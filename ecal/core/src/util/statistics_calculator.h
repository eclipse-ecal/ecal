/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
 * @brief This file provides classes to calculate statistics.
 *        These classes are NOT threadsafe!
**/

#pragma once

#include <ecal_struct_sample_registration.h>
#include <algorithm>

namespace eCAL
{
  class StatisticsCalculator
  {
  public:
    void Update(double value)
    {
      statistics_.count++;
      statistics_.latest = value;
      statistics_.min = std::min(statistics_.min, value);
      statistics_.max = std::max(statistics_.max, value);
      const double delta = value - statistics_.mean;
      statistics_.mean += delta / static_cast<double>(statistics_.count);
      const double delta2 = value - statistics_.mean;
      statistics_.variance += delta * delta2;
    }

    Registration::Statistics GetStatistics() const
    {
      return statistics_;
    }
  private:
    Registration::Statistics statistics_;
  };
}





