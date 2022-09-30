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

#include "utils.h"
#include "measurement_importer.h"
#include "measurement_exporter.h"

class MeasurementConverter
{
public:
  MeasurementConverter();

  void setCurrentJob(eCALMeasCutterUtils::MeasurementJob& current_job);
  bool isConfigurationValid();
  bool convert();
  void abort();

private:
  std::pair<eCALMeasCutterUtils::Timestamp, eCALMeasCutterUtils::Timestamp> getCalculatedStartEndTimestamps();
  double                                                                    getConversionFactor(const eCALMeasCutterUtils::ScaleType scale_type);
  bool isChannelMentionedInFile(const std::string& channel_name);
  eCALMeasCutterUtils::MeasurementJob                                       _current_job;
  bool                                                                      _abort_conversion;
  bool                                                                      _is_channel_manipulation_valid;

  MeasurementImporter                                                       _importer;
  MeasurementExporter                                                       _exporter;

  eCALMeasCutterUtils::Timestamp                                            _original_start_timestamp;
  eCALMeasCutterUtils::Timestamp                                            _original_end_timestamp;

  eCALMeasCutterUtils::Timestamp                                            _calculated_start_timestamp;
  eCALMeasCutterUtils::Timestamp                                            _calculated_end_timestamp;

};

