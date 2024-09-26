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

#include <monitoring/ecal_monitoring_filter.h>

#include <mutex>
#include <set>

#include <ecal/ecal_os.h>

#ifdef ECAL_OS_LINUX
#include <strings.h>  // strcasecmp
#endif

namespace
{
  struct InsensitiveCompare
  {
    bool operator() (const std::string& a, const std::string& b) const
    {
#ifdef ECAL_OS_WINDOWS
      return _stricmp(a.c_str(), b.c_str()) < 0;
#endif
#ifdef ECAL_OS_LINUX
      return strcasecmp(a.c_str(), b.c_str()) < 0;
#endif
    }
  };

  using StrICaseSetT = std::set<std::string, InsensitiveCompare>;

  void Tokenize(const std::string& str, StrICaseSetT& tokens, const std::string& delimiters, bool trimEmpty)
  {
    std::string::size_type pos = 0;
    std::string::size_type lastPos = 0;

    for (;;)
    {
      pos = str.find_first_of(delimiters, lastPos);
      if (pos == std::string::npos)
      {
        pos = str.length();
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace(str.data() + lastPos, pos - lastPos);
        }
        break;
      }
      else
      {
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace(str.data() + lastPos, pos - lastPos);
        }
      }
      lastPos = pos + 1;
    }
  }

  std::vector<std::regex> CreateRegexVector(const std::string& filter_)
  {
    std::vector<std::regex> regex_vector;
    StrICaseSetT compare_set;
    Tokenize(filter_, compare_set, ",;", true);
    for (const auto& it : compare_set)
    {
      regex_vector.emplace_back(it, std::regex::icase);
    }
    return regex_vector;
  }

  bool MatchAnyRegex(const std::string& topic_name, const std::vector<std::regex>& regexes)
  {
    for (const auto& regex : regexes)
    {
      if (std::regex_match(topic_name, regex))
      {
        return true;
      }
    }
    return false;
  }
}



eCAL::CMonitoringFilter::CMonitoringFilter(const Monitoring::SAttributes& attr_)
  : m_attributes(attr_)
{
  ActivateFilter();
}

void eCAL::CMonitoringFilter::SetExclFilter(const std::string& filter_)
{
  m_attributes.filter_excl = filter_;
}

void eCAL::CMonitoringFilter::SetInclFilter(const std::string& filter_)
{
  m_attributes.filter_incl = filter_;
}

bool eCAL::CMonitoringFilter::AcceptTopic(const std::string& topic_name) const
{
  // topics are rejected if:
  // a) they are matched by the exclude list
  // b) there exists an include list, and they are not in the include list
  // topics are accepted if they are not rejected.

  const bool reject_because_excluded = MatchAnyRegex(topic_name, m_exclude_filters);
  if (reject_because_excluded)
    return false;

  if (!m_include_filters.empty())
  {
    const bool topic_is_included = MatchAnyRegex(topic_name, m_include_filters);
    if (!topic_is_included)
      return false;
  }

  return true;
}

void eCAL::CMonitoringFilter::ActivateFilter()
{
  m_exclude_filters = CreateRegexVector(m_attributes.filter_excl);
  m_include_filters = CreateRegexVector(m_attributes.filter_incl);
}

void eCAL::CMonitoringFilter::DeactivateFilter()
{
  m_exclude_filters.clear();
  m_include_filters.clear();
}