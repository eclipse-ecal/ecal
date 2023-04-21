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
#include <algorithm>
#include <regex>
#include <cctype>
#include <array>

#include <ecal_utils/filesystem.h>
#include <ecalhdf5/eh5_meas.h>
#include <ecalhdf5/eh5_reader.h>

#include "utils.h"

class MeasurementImporter
{
public:
  MeasurementImporter();
  ~MeasurementImporter();
  MeasurementImporter(MeasurementImporter const&) = delete;
  MeasurementImporter& operator =(MeasurementImporter const&) = delete;
  MeasurementImporter(MeasurementImporter&&) = delete;
  MeasurementImporter& operator=(MeasurementImporter&&) = delete;
  void                                                                                    setPath(const std::string& path);
  eCALMeasCutterUtils::ChannelNameSet                                                     getChannelNames() const;
  bool                                                                                    hasChannel(const std::string& channel_name) const;
  void                                                                                    openChannel(const std::string& channel_name);
  eCALMeasCutterUtils::ChannelInfo                                                        getChannelInfoforCurrentChannel() const;
  eCALMeasCutterUtils::TimestampSet                                                       getTimestamps() const;
  void                                                                                    getData(eCALMeasCutterUtils::Timestamp timestamp, eCALMeasCutterUtils::MetaData& meta_data, std::string& data);
  std::pair<eCALMeasCutterUtils::Timestamp, eCALMeasCutterUtils::Timestamp>               getOriginalStartFinishTimestamps();
  std::list<std::string>                                                                  getChannelNamesForRegex(const std::regex& regex);
  std::string                                                                             getLoadedPath();
  bool                                                                                    areChannelsLeftToInclude(const std::list<std::string>& channels_to_exclude);

private:
  bool                                 isEcalMeasFile(const std::string& path);
  bool                                 isProtoChannel(const std::string& channel_type);
  std::unique_ptr<eCAL::measurement::base::Reader>      _reader;
  eCALMeasCutterUtils::ChannelData                      _current_opened_channel_data;
  std::string                                           _loaded_path;
  eCALMeasCutterUtils::ChannelNameSet                   _channel_names;
};

class ImporterException : public std::exception
{
public:
  ImporterException(const std::string& message) : message_(message) {}

  const char* what() const throw() override { return message_.c_str(); }
private:
  std::string message_;
};

