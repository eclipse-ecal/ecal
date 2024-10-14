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
 * @brief  Filter that checks if a topic / service is to be listed in the monitoring.
**/

#include <regex>
#include <string>
#include <vector>

#include "monitoring/config/attributes/monitoring_attributes.h"

namespace eCAL
{
  class CMonitoringFilter
  {
  public:
    CMonitoringFilter(const Monitoring::SAttributes& attr_);

    // Sets the filters. The user needs to call activate filter, after setting them.
    void SetExclFilter(const std::string& filter_);
    void SetInclFilter(const std::string& filter_);

    // Returns true if topic is accepted by the filter, false otherwise
    bool AcceptTopic(const std::string& topic_name) const;

    void ActivateFilter();
    void DeactivateFilter();

  private:
    Monitoring::SAttributes m_attributes;
    std::vector<std::regex> m_include_filters;
    std::vector<std::regex> m_exclude_filters;
  };
}