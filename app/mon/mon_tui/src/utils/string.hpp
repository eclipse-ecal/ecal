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

#include <string>
#include <vector>
#include <cctype>
#include <sstream>
#include <iomanip>

#include "model/data/topic.hpp"
#include "model/data/process.hpp"
#include "model/log.hpp"

std::vector<std::string> SplitWords(const std::string& str)
{
  std::vector<std::string> words;
  std::string w;

  // looping one-past-the-end to process a word that ends at size()-1
  for (size_t i = 0; i <= str.size(); ++i)
  {
    if (i < str.size() && std::isalpha(str[i]))
        w += str[i];
    else if (!w.empty())
    {
        words.push_back(w);
        w.clear();
    }
  }

  return words;
}

bool StartsWith(const std::string &str, const std::string &start)
{
  return str.substr(0, start.size()) == start;
}

std::string EcalTimestampToString(int64_t val)
{
  //This is probably not universaly portable, might fail if system typedefs time_t differently
  std::time_t time = val/(1000 * 1000);
  auto tm = std::gmtime(&time);
  std::stringstream ss;
  ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
  return ss.str();
}

namespace std
{

std::string to_string(LogEntry::Level val)
{
  switch (val)
  {
    case LogEntry::Level::DEBUG1:
      return "Debug 1";
    case LogEntry::Level::DEBUG2:
      return "Debug 2";
    case LogEntry::Level::DEBUG3:
      return "Debug 3";
    case LogEntry::Level::DEBUG4:
      return "Debug 4";
    case LogEntry::Level::INFO:
      return "Info";
    case LogEntry::Level::WARNING:
      return "Warning";
    case LogEntry::Level::ERROR:
      return "Error";
    case LogEntry::Level::FATAL:
      return "Fatal";
    default:
      return "Unknown";
  }
}

std::string to_string(Process::TimeSyncState value)
{
  switch(value)
  {
    case Process::TimeSyncState::REALTIME: return "realtime";
    case Process::TimeSyncState::REPLAY: return "replay";
    case Process::TimeSyncState::NONE:
    default:
      return "None";
  }
}

std::string to_string(Process::Severity value)
{
  switch(value)
  {
    case Process::Severity::HEALTHY:
      return "Healthy";
    case Process::Severity::WARNING:
      return "Warning";
    case Process::Severity::CRITICAL:
      return "Critical";
    case Process::Severity::FAILED:
      return "Failed";
    case Process::Severity::UNKNOWN:
    default:
      return "Unknown";
  }
}

std::string to_string(Topic::TransportLayer value)
{
  switch(value)
  {
    case Topic::TransportLayer::UDP_MC: return "UDP multicast";
    case Topic::TransportLayer::SHM: return "SHM";
    case Topic::TransportLayer::TCP: return "TCP";
    case Topic::TransportLayer::INPROC: return "InProc";
    case Topic::TransportLayer::ALL: return "All";
    case Topic::TransportLayer::NONE:
    default:
      return "None";
  }
}

std::string to_string(Topic::Direction value)
{
  switch(value)
  {
    case Topic::Direction::PUBLISHER: return "publisher";
    case Topic::Direction::SUBSCRIBER:
    default:
      return "subscriber";
  }
}

std::string to_string(const std::vector<Topic::TransportLayer> &value)
{
  std::string str;
  for(auto v: value)
  {
    str += to_string(v) + " ";
  }
  return str;
}

std::string to_string(Topic::QOSHistoryKind value)
{
  switch(value)
  {
    case Topic::QOSHistoryKind::KEEP_LAST: return "Keep last";
    case Topic::QOSHistoryKind::KEEP_ALL:
    default:
      return "Keep all";
  }
}

std::string to_string(Topic::QOSReliability value)
{
  switch(value)
  {
    case Topic::QOSReliability::RELIABLE: return "Reliable";
    case Topic::QOSReliability::BEST_EFFORT:
    default:
      return "Best effort";
  }
}

}
