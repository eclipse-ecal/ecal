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

#include "measurement_converter.h"

MeasurementConverter::MeasurementConverter() :
  _abort_conversion(false),
  _is_channel_manipulation_valid(true),
  _original_start_timestamp(std::numeric_limits<eCALMeasCutterUtils::Timestamp>::min()),
  _original_end_timestamp(std::numeric_limits<eCALMeasCutterUtils::Timestamp>::max()),
  _calculated_start_timestamp(std::numeric_limits<eCALMeasCutterUtils::Timestamp>::min()),
  _calculated_end_timestamp(std::numeric_limits<eCALMeasCutterUtils::Timestamp>::max())
{
}

void MeasurementConverter::setCurrentJob(eCALMeasCutterUtils::MeasurementJob& current_job)
{
  _current_job = current_job;
}

bool MeasurementConverter::isConfigurationValid()
{
  // check if input measurement exists and available
  auto file_status = EcalUtils::Filesystem::FileStatus(_current_job.input_measurement_path, EcalUtils::Filesystem::OsStyle::Current);
  if (!file_status.IsOk())
  {
    eCALMeasCutterUtils::printError("Failed loading measurement from \"" + _current_job.input_measurement_path + "\": Resource unavailable.", _current_job.id);
    return false;
  }

  // check if output measurement path exists and available
  file_status = EcalUtils::Filesystem::FileStatus(_current_job.output_measurement_path, EcalUtils::Filesystem::OsStyle::Current);
  if (!file_status.IsOk())
  {
    eCALMeasCutterUtils::printError("Output directory from \"" + _current_job.output_measurement_path + "\": Resource unavailable.", _current_job.id);
    return false;
  }

  // check if output directory is really a directory
  if (file_status.GetType() != EcalUtils::Filesystem::Type::Dir)
  {
    eCALMeasCutterUtils::printError("Output directory from \"" + _current_job.output_measurement_path + "\": It has to be a directory.", _current_job.id);
    return false;
  }

  // check if input measurement can be loaded
  try
  {
    _importer.setPath(_current_job.input_measurement_path);
  }
  catch (const ImporterException& e)
  {
    eCALMeasCutterUtils::printError(e.what(), _current_job.id);
    return false;
  }

  // check if output writer can be loaded
  try
  {
    _exporter.setPath(_current_job.output_measurement_path, _current_job.basename, _current_job.max_size_per_file);
  }
  catch (const ExporterException& e)
  {
    eCALMeasCutterUtils::printError(e.what(), _current_job.id);
    return false;
  }

  //check if output directory is empty
  auto actual_output_path = _exporter.getOutputPath();
  if (!eCALMeasCutterUtils::isDirectoryEmpty(actual_output_path))
  {
    eCALMeasCutterUtils::printError("Output directory from \"" + actual_output_path + "\": Directory not empty.", _current_job.id);
    return false;
  }

  // check if channels regex for include/exclude list exist in the measurement
  for (const auto& channel_regex : _current_job.channels_to_manipulate_regex)
  {
    auto channels_matches = _importer.getChannelNamesForRegex(std::regex(channel_regex));
    if (channels_matches.empty())
    {
      eCALMeasCutterUtils::printError("No match for channel regex \"" + channel_regex + "\" in measurement \"" + _current_job.input_measurement_path + "\".", _current_job.id);
    }
    else
    {
      _current_job.channels_to_manipulate.insert(_current_job.channels_to_manipulate.end(), channels_matches.begin(), channels_matches.end());
    }
  }

  auto at_least_one_channel_in_measurement = false;
  // check if channels for include/exclude list exist in the measurement
  for (const auto& channel : _current_job.channels_to_manipulate)
  {
    if (!_importer.hasChannel(channel))
    {
      eCALMeasCutterUtils::printError("Channel  \"" + channel + "\" : Does not exist in measurement \"" + _current_job.input_measurement_path + "\".", _current_job.id);
    }
    else
    {
      at_least_one_channel_in_measurement = true;
    }
  }

  // check if we have any channels to manipulate
  if (_current_job.channels_to_manipulate.empty() || !at_least_one_channel_in_measurement)
  {
    _is_channel_manipulation_valid = false;
    eCALMeasCutterUtils::printError("No channels to manipulate in measurement \"" + _current_job.input_measurement_path + "\".", _current_job.id);
  }

  if (_current_job.operation_type == eCALMeasCutterUtils::ChannelOperationType::exclude && !_importer.areChannelsLeftToInclude(_current_job.channels_to_manipulate))
  {
    _is_channel_manipulation_valid = false;
    eCALMeasCutterUtils::printError("No channels left to include for \"" + _current_job.input_measurement_path + "\".", _current_job.id);
  }

  auto is_trimming_valid = true;
  // check if calculated start time and end time are in bounds of measurement
  std::tie(_original_start_timestamp, _original_end_timestamp) = _importer.getOriginalStartFinishTimestamps();
  std::tie(_calculated_start_timestamp, _calculated_end_timestamp) = getCalculatedStartEndTimestamps();

  // if the trimming is not valid, there is no point to compare original timestamps with the calculated ones
  // we will use the original ones since the one mentioned in the file is not valid
  if (!_current_job.trim_operation._is_valid)
    is_trimming_valid = false;

  // check if the calculated start time is lower than the original start time
  if (_calculated_start_timestamp < _original_start_timestamp)
  {
    eCALMeasCutterUtils::printError("Start time is set before the original start time for measurement \"" + _current_job.input_measurement_path + "\".", _current_job.id);
    is_trimming_valid = false;
  }

  // check if the calculated start time is bigger than the original end time
  if (_calculated_start_timestamp > _original_end_timestamp)
  {
    eCALMeasCutterUtils::printError("Start time is set after the original end time for measurement \"" + _current_job.input_measurement_path + "\".", _current_job.id);
    is_trimming_valid = false;
  }

  // check if the calculated end time is bigger than the original end time
  if (_calculated_end_timestamp > _original_end_timestamp)
  {
    eCALMeasCutterUtils::printError("End time is set after the original end time for measurement \"" + _current_job.input_measurement_path + "\".", _current_job.id);
    is_trimming_valid = false;
  }

  // check if calculated end time is lower than original start time
  if (_calculated_end_timestamp < _original_start_timestamp)
  {
    eCALMeasCutterUtils::printError("End time is set before the original start time for measurement \"" + _current_job.input_measurement_path + "\".", _current_job.id);
    is_trimming_valid = false;
  }

  // check that calculated start time is before calculated end time
  if (_calculated_end_timestamp < _calculated_start_timestamp)
  {
    eCALMeasCutterUtils::printError("End time is set before start time for measurement \"" + _current_job.input_measurement_path + "\".", _current_job.id);
    is_trimming_valid = false;
  }

  // if the trimming is not in bounds, use the original ones
  if (!is_trimming_valid)
  {
    std::tie(_calculated_start_timestamp, _calculated_end_timestamp) = _importer.getOriginalStartFinishTimestamps();
  }

  if ((!is_trimming_valid || (!_current_job.trim_operation._is_start_set && !_current_job.trim_operation._is_end_set)) && !_is_channel_manipulation_valid && !_current_job.basename_mentioned_in_config && !_current_job.max_size_mentioned_in_config)
  {
    eCALMeasCutterUtils::printError("No valid operation was set for measurement \"" + _current_job.input_measurement_path + "\".", _current_job.id);
    return false;
  }

  return true;
}

void MeasurementConverter::abort()
{
  _abort_conversion = true;
}

bool MeasurementConverter::convert()
{
  bool conversion_result = true;

  std::string payload;
  eCALMeasCutterUtils::MetaData meta_data;

  auto channel_names = _importer.getChannelNames();

  eCALMeasCutterUtils::printOutput("Processing " + _importer.getLoadedPath() + " as " + _current_job.id + "\n" 
                                   + std::string(13, ' ') + "Exporting to " + _exporter.getOutputPath());

  for (const auto& channel_name : channel_names)
  {
    if (_abort_conversion)
    {
      conversion_result = false;
      break;
    }
    if (_is_channel_manipulation_valid)
    {
      if (_current_job.operation_type == eCALMeasCutterUtils::ChannelOperationType::exclude &&
        isChannelMentionedInFile(channel_name))  continue;

      if (_current_job.operation_type == eCALMeasCutterUtils::ChannelOperationType::include &&
        !isChannelMentionedInFile(channel_name))  continue;
    }

    try
    {
      _importer.openChannel(channel_name);
      auto channel_info = _importer.getChannelInfoforCurrentChannel();
      auto timestamps = _importer.getTimestamps();
      auto timestamp_begin_iter = timestamps.lower_bound(_calculated_start_timestamp);
      auto timestamp_end_iter = timestamps.upper_bound(_calculated_end_timestamp);

      _exporter.createChannel(channel_name, channel_info);

      eCALMeasCutterUtils::printOutput("Exporting channel " + channel_name + "...", _current_job.id);

      for (auto timestamp_iter = timestamp_begin_iter; timestamp_iter != timestamp_end_iter; ++timestamp_iter)
      {
        if (_abort_conversion)
        {
          conversion_result = false;
          break;
        }
        _importer.getData(*timestamp_iter, meta_data, payload);
        _exporter.setData((*timestamp_iter), meta_data, payload);
      }
    }
    catch (const ImporterException& e)
    {
      eCALMeasCutterUtils::printError("Importing error in channel " + channel_name + ": " + e.what(), _current_job.id);
      conversion_result = false;
    }
    catch (const ExporterException& e)
    {
      eCALMeasCutterUtils::printError("Exporting error in channel: " + channel_name + ": " + e.what(), _current_job.id);
      conversion_result = false;
    }
    catch (const std::bad_alloc& e)
    {
      conversion_result = false;
      eCALMeasCutterUtils::printError("Memory limit has been exceeded: " + std::string(e.what()), _current_job.id);
    }
    catch (...)
    {
      conversion_result = false;
      eCALMeasCutterUtils::printError("An unknown error has occured in channel " + channel_name + ". Please report this to an AT9 team member.", _current_job.id);
    }
  }

  auto input_path  = EcalUtils::Filesystem::ToNativeSeperators(EcalUtils::Filesystem::CleanPath(_importer.getLoadedPath()));
  auto output_path = EcalUtils::Filesystem::ToNativeSeperators(EcalUtils::Filesystem::CleanPath(_exporter.getRootOutputPath()));

  auto file_status = EcalUtils::Filesystem::FileStatus(input_path, EcalUtils::Filesystem::OsStyle::Current);
  if (file_status.GetType() == EcalUtils::Filesystem::Type::Dir)
  {
    for (const auto& file : EcalUtils::Filesystem::DirContent(_importer.getLoadedPath()))
    {
      //copy .ecalmeas file
      if (eCALMeasCutterUtils::GetExtension(file.first) == "ecalmeas")
      {
        auto source      = input_path + EcalUtils::Filesystem::NativeSeparator(EcalUtils::Filesystem::OsStyle::Current) + file.first;
        auto destination = output_path + EcalUtils::Filesystem::NativeSeparator(EcalUtils::Filesystem::OsStyle::Current) + file.first;

        if(EcalUtils::Filesystem::CopyFile(source, destination, EcalUtils::Filesystem::OsStyle::Current))
          eCALMeasCutterUtils::printOutput("Finished copying file \"" + file.first + "\" to \"" + output_path + "\".", _current_job.id);
        else
          eCALMeasCutterUtils::printError("Could not copy file \"" + file.first + "\" to \"" + output_path + "\".", _current_job.id);
      }

      // copy doc folder
      if (!file.first.compare("doc"))
      {
        auto doc_input_folder_path  = input_path + EcalUtils::Filesystem::NativeSeparator(EcalUtils::Filesystem::OsStyle::Current) + file.first;
        auto doc_output_folder_path = output_path + EcalUtils::Filesystem::NativeSeparator(EcalUtils::Filesystem::OsStyle::Current) + file.first;
        if (EcalUtils::Filesystem::MkDir(doc_output_folder_path))
        {
          // iterate through doc folder and copy everything
          for (const auto& doc_file : EcalUtils::Filesystem::DirContent(doc_input_folder_path))
          {
            if (doc_file.second.GetType() == EcalUtils::Filesystem::Type::RegularFile)
            {
              auto source = doc_input_folder_path + EcalUtils::Filesystem::NativeSeparator(EcalUtils::Filesystem::OsStyle::Current) + doc_file.first;
              auto destination = doc_output_folder_path + EcalUtils::Filesystem::NativeSeparator(EcalUtils::Filesystem::OsStyle::Current) + doc_file.first;

              if (EcalUtils::Filesystem::CopyFile(source, destination, EcalUtils::Filesystem::OsStyle::Current))
                eCALMeasCutterUtils::printOutput("Finished copying file \"" + doc_file.first + "\" to \"" + doc_output_folder_path + "\".", _current_job.id);
              else
                eCALMeasCutterUtils::printError("Could not copy file \"" + doc_file.first + "\" to \"" + doc_output_folder_path + "\".", _current_job.id);
            }
          }
        }
      }
    }
  }
 
  if (conversion_result)
    eCALMeasCutterUtils::printOutput("Done processing " + _importer.getLoadedPath());
  return conversion_result;
}

std::pair<eCALMeasCutterUtils::Timestamp, eCALMeasCutterUtils::Timestamp> MeasurementConverter::getCalculatedStartEndTimestamps()
{
  std::pair<eCALMeasCutterUtils::Timestamp, eCALMeasCutterUtils::Timestamp> start_stop_pair(_original_start_timestamp, _original_end_timestamp);

  if (!_current_job.trim_operation._is_valid)
    return start_stop_pair;

  if (_current_job.trim_operation._is_start_set)
  {
    double conversion_factor = getConversionFactor(_current_job.trim_operation._start_scale_type);

    switch (_current_job.trim_operation._start_base_type)
    {
    case(eCALMeasCutterUtils::BaseType::start):
    {
      start_stop_pair.first = _original_start_timestamp + (eCALMeasCutterUtils::Timestamp)(_current_job.trim_operation._start_time * conversion_factor);
      break;
    }
    case(eCALMeasCutterUtils::BaseType::end):
    {
      start_stop_pair.first = _original_end_timestamp - (eCALMeasCutterUtils::Timestamp)(_current_job.trim_operation._start_time * conversion_factor);
      break;
    }
    case(eCALMeasCutterUtils::BaseType::absolute):
    {
      start_stop_pair.first = (eCALMeasCutterUtils::Timestamp)(_current_job.trim_operation._start_time * conversion_factor);
      break;
    }
    default:
      break;
    }
  }

  if (_current_job.trim_operation._is_end_set)
  {
    double conversion_factor = getConversionFactor(_current_job.trim_operation._end_scale_type);

    switch (_current_job.trim_operation._end_base_type)
    {
    case(eCALMeasCutterUtils::BaseType::start):
    {
      start_stop_pair.second = _original_start_timestamp + (eCALMeasCutterUtils::Timestamp)(_current_job.trim_operation._end_time * conversion_factor);
      break;
    }
    case(eCALMeasCutterUtils::BaseType::end):
    {
      start_stop_pair.second = _original_end_timestamp - (eCALMeasCutterUtils::Timestamp)(_current_job.trim_operation._end_time * conversion_factor);
      break;
    }
    case(eCALMeasCutterUtils::BaseType::absolute):
    {
      start_stop_pair.second = (eCALMeasCutterUtils::Timestamp)(_current_job.trim_operation._end_time * conversion_factor);
      break;
    }
    default:
      break;
    }
  }
  return start_stop_pair;
}

double MeasurementConverter::getConversionFactor(const eCALMeasCutterUtils::ScaleType scale_type)
{
  switch (scale_type)
  {
  case(eCALMeasCutterUtils::ScaleType::seconds):
  {
    return 1000000.0;
    break;
  }
  case(eCALMeasCutterUtils::ScaleType::milliseconds):
  {
    return 1000.0;
    break;
  }
  case(eCALMeasCutterUtils::ScaleType::microseconds):
  {
    return 1.0;
    break;
  }
  default:
    return 1.0;
    break;
  }
}

bool MeasurementConverter::isChannelMentionedInFile(const std::string& channel_name)
{
  auto result = std::find(std::begin(_current_job.channels_to_manipulate), std::end(_current_job.channels_to_manipulate), channel_name) != std::end(_current_job.channels_to_manipulate);
  return result;
}

