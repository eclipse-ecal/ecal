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
#include <unordered_map>
#include <map>

#include <ecalhdf5/eh5_writer.h>
#include <ecal_utils/filesystem.h>
#include "utils.h"

class MeasurementExporter
{
public:
  MeasurementExporter();
  ~MeasurementExporter();
  MeasurementExporter(MeasurementExporter const&) = delete;
  MeasurementExporter& operator =(MeasurementExporter const&) = delete;
  MeasurementExporter(MeasurementExporter&&) = delete;
  MeasurementExporter& operator=(MeasurementExporter&&) = delete;

  void        setPath(const std::string& path, const std::string& base_name, const size_t& max_size_per_file);
  void        createChannel(const std::string& channel_name, const eCALMeasCutterUtils::ChannelInfo& channel_info);
  void        setData(eCALMeasCutterUtils::Timestamp timestamp, const eCALMeasCutterUtils::MetaData& meta_data, const std::string& payload);
  std::string getOutputPath();

private:
  std::unique_ptr<eCAL::measurement::base::Writer>      _writer;
  std::string                                           _current_channel_name;
  std::string                                           _output_path;
};

class ExporterException : public std::exception
{
public:
  ExporterException(const std::string& message) : message_(message) {}

  const char* what() const throw() override { return message_.c_str(); }
private:
  std::string message_;
};

