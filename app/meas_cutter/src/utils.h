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

#include <iostream>
#include <chrono>
#include <iomanip>
#include <unordered_map>
#include <fstream>
#include <algorithm>

#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"

#include <ecalhdf5/eh5_meas.h>
#include <ecal_utils/str_convert.h>
#include <ecal_utils/filesystem.h>

#include "logger.h"

namespace eCALMeasCutterUtils
{
  typedef long long Timestamp;
  typedef std::set<Timestamp> TimestampSet;

  constexpr const char* kDefaultHdf5BaseName     = "measurement";
  constexpr const int  kDefaultHdf5FileSize      = 512;
  constexpr const char* kDefaultFolderOutput     = "MEASUREMENT_CONVERTER";
  constexpr const char* kDefaultLogOutputFolder  = "log";
  
  extern bool quiet;
  extern bool save_log;
  extern bool enable_one_file_per_topic;

  static std::fstream log_file_output_stream;
  static std::string getLogTime()
  {
    const auto real_time_clock      = std::chrono::system_clock::now();

    const auto current_time_t       = std::chrono::system_clock::to_time_t(real_time_clock);
    const auto current_localtime    = *std::localtime(&current_time_t);
   
    const auto current_time_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (real_time_clock.time_since_epoch()).count() % 1000;

    std::ostringstream output;
    output << std::put_time(&current_localtime, "%T") << ":" << std::setw(3) << std::setfill('0') << current_time_milliseconds;
    return output.str();
  }

  static void appendToLogFile(const std::string& output)
  {
    Logger::append(output);
  }

  static inline void printError(const std::string& output, const std::string& id = "")
  {
    std::string output_text;
    if (id.empty())
    {
      output_text = getLogTime() + " " + output + "\n";
    }
    else
    {
      output_text = getLogTime() + " " + id + " : " + output + "\n";
    }
    std::cerr << output_text;

    if (save_log)
      appendToLogFile(output_text);
  }

  static inline void printOutput(const std::string& output, const std::string& id = "")
  {
    std::string output_text;
    if (id.empty())
    {
      output_text = getLogTime() + " " + output + "\n";
    }
    else
    {
      output_text = getLogTime() + " " + id + " : " + output + "\n";
    }

    if (!quiet)
    {
      std::cout << output_text;
    }

    if (save_log)
      appendToLogFile(output_text);
  }


  static inline void createLogFile()
  {
    auto output_dir = EcalUtils::Filesystem::ApplicationDir() + EcalUtils::Filesystem::NativeSeparator(EcalUtils::Filesystem::OsStyle::Current) + kDefaultLogOutputFolder;

    auto file_status = EcalUtils::Filesystem::FileStatus(output_dir, EcalUtils::Filesystem::OsStyle::Current);

    if (!file_status.IsOk())
      if (!EcalUtils::Filesystem::MkDir(output_dir))
        eCALMeasCutterUtils::printError("Could not open directory \"" + output_dir + "\".");

    auto timestamp = getLogTime();
    std::replace(timestamp.begin(), timestamp.end(), ':', '-');
    auto log_file_name = output_dir + EcalUtils::Filesystem::NativeSeparator(EcalUtils::Filesystem::OsStyle::Current) + timestamp + ".log";
    
    Logger::openFile(log_file_name);
  }

  static inline bool isDirectoryEmpty(const std::string& path_to_directory)
  {
    return EcalUtils::Filesystem::DirContent(path_to_directory).empty();
  }

  static inline std::string GetExtension(const std::string& path)
  {
    size_t index_extension = path.find_last_of('.');
    if (index_extension != std::string::npos)
    {
      return path.substr(index_extension + 1);
    }
    return "";
  }

  enum class ScaleType
  {
    null, seconds, microseconds, milliseconds
  };

  enum class BaseType
  {
    null, start, end, absolute
  };

  static std::unordered_map<std::string, ScaleType> const scale_type_table = { {"null", ScaleType::null}, {"s", ScaleType::seconds}, {"seconds", ScaleType::seconds}, {"second", ScaleType::seconds },
                                                                             {"us", ScaleType::microseconds}, {"microseconds", ScaleType::microseconds},  {"microsecond", ScaleType::microseconds},
                                                                             {"ms", ScaleType::milliseconds}, {"millisecond", ScaleType::milliseconds}, {"milliseconds", ScaleType::milliseconds} };

  static std::unordered_map<std::string, BaseType> const base_type_table = { {"null", BaseType::null}, {"start", BaseType::start}, {"end",BaseType::end}, {"absolute", BaseType::absolute} };

  struct TrimOperation
  {
    bool             _is_valid;
    bool             _is_start_set;
    bool             _is_end_set;
    bool             _is_start_scale_valid;
    bool             _is_end_scale_valid;

    // variable used to know if any trimming is specified but not valid
    // or if no trimming is desired at all
    bool             _is_start_trimming_specified;
    bool             _is_end_trimming_specified;

    double           _start_time;
    double           _end_time;

    ScaleType        _start_scale_type;
    ScaleType        _end_scale_type;
    BaseType         _start_base_type;
    BaseType         _end_base_type;


    TrimOperation() :
      _is_valid(true),
      _is_start_set(false),
      _is_end_set(false),
      _is_start_scale_valid(true),
      _is_end_scale_valid(true),
      _is_start_trimming_specified(false),
      _is_end_trimming_specified(false),
      _start_time(std::numeric_limits<double>::min()),
      _end_time(std::numeric_limits<double>::max()),
      _start_scale_type(ScaleType::seconds),
      _end_scale_type(ScaleType::seconds),
      _start_base_type(BaseType::null),
      _end_base_type(BaseType::null)
    {}

    friend void operator>>(const YAML::Node& node, TrimOperation& trim)
    {
      try
      {
        if (node["start"])
        {
          trim._is_start_trimming_specified = true;

          if (node["start"]["time"])
          {
            trim._start_time = node["start"]["time"].as<double>();
          }

          if (node["start"]["scale"])
          {
            auto it = scale_type_table.find(node["start"]["scale"].as<std::string>());

            if (it != scale_type_table.end())
              trim._start_scale_type = it->second;
            else
              trim._is_start_scale_valid = false;
          }
          if (node["start"]["base"])
          {
            auto it = base_type_table.find(node["start"]["base"].as<std::string>());

            if (it != base_type_table.end())
              trim._start_base_type = it->second;
          }
        }

        if (node["end"])
        {
          trim._is_end_trimming_specified = true;

          if (node["end"]["time"])
          {
            trim._end_time = node["end"]["time"].as<double>();
          }

          if (node["end"]["scale"])
          {
            auto it = scale_type_table.find(node["end"]["scale"].as<std::string>());

            if (it != scale_type_table.end())
              trim._end_scale_type = it->second;
            else
              trim._is_end_scale_valid = false;
          }
          if (node["end"]["base"])
          {
            auto it = base_type_table.find(node["end"]["base"].as<std::string>());

            if (it != base_type_table.end())
              trim._end_base_type = it->second;
          }
        }
      }
      catch (const YAML::BadConversion& e)
      {
        printError(e.what());
        std::exit(EXIT_FAILURE);
      }
    }
  };

  enum class ChannelOperationType
  {
    null, include, exclude
  };

  struct MeasurementJob
  {
    std::string            id;
    TrimOperation          trim_operation;
    std::string            input_measurement_path;
    std::string            output_measurement_path;

    std::list<std::string> channels_to_manipulate;
    std::list<std::string> channels_to_manipulate_regex;
    ChannelOperationType   operation_type;
    std::string            basename;
    size_t                 max_size_per_file;

    bool                   basename_mentioned_in_config;
    bool                   max_size_mentioned_in_config;

    MeasurementJob():
    operation_type(ChannelOperationType::null),
    basename(kDefaultHdf5BaseName),
    max_size_per_file(kDefaultHdf5FileSize),
    basename_mentioned_in_config(false),
    max_size_mentioned_in_config(false)
    {}
  };

  enum class MetaDatumKey
  {
    RECEIVER_TIMESTAMP,
    SENDER_TIMESTAMP,
    SENDER_ID,
    SENDER_CLOCK
  };

  struct MetaDatumHash
  {
    template <typename T>
    std::size_t operator()(T t) const 
    {
      return static_cast<std::size_t>(t);
    }
  };

  union MetaDatumValue
  {
    Timestamp receiver_timestamp;
    Timestamp sender_timestamp;
    uint64_t sender_id;
    uint64_t sender_clock;

      std::array<char,64> __union_size;
  };

  typedef std::unordered_map<MetaDatumKey, MetaDatumValue, MetaDatumHash> MetaData;
  typedef std::set<std::string> ChannelNameSet;

  enum class SerializationFormat
  {
    UNKNOWN,
    PROTOBUF
  };

  struct ChannelInfo
  {
    SerializationFormat format;
    std::string type;
    std::string description;
    std::string name;
    std::string __reserved_field_2;
  };

  struct ChannelData
  {
    ChannelInfo _channel_info;
    TimestampSet _timestamps;
    std::unordered_map<Timestamp, eCAL::measurement::base::EntryInfo> _timestamp_entry_info_map;
  };
}
