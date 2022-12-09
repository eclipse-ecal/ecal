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

#include "ecal_meas_cutter.h"

bool eCALMeasCutterUtils::quiet                     = false;
bool eCALMeasCutterUtils::save_log                  = false;
bool eCALMeasCutterUtils::enable_one_file_per_topic = false;

eCALMeasCutter::eCALMeasCutter(std::vector<std::string>& arguments):
  _max_size_per_file(0),
  _is_base_name_mention_in_config(false),
  _is_max_file_size_mention_in_config(false)
{
  parseCmdLine(arguments);
}

void eCALMeasCutter::parseCmdLine(std::vector<std::string>& arguments)
{
  TCLAP::CmdLine cmd("eCAL Measurement Cutter", ' ', EcalMeasCutterGlobals::VERSION_STRING);

  TCLAP::ValueArg<std::string> config_file_path_arg("c", "config", "Path to the configuration file that contains operations to be done on the measurements.", true, "", "string", cmd);
  TCLAP::MultiArg<std::string> input_measurement_arg("i", "input", "Path of the input measurement (Tip: one input measurement has to have a corresponding output).", true, "string", cmd);
  TCLAP::MultiArg<std::string> output_measurement_arg("o", "output", "Path of the output measurement (Tip: one input measurement has to have a corresponding output).", true, "string", cmd);
  TCLAP::SwitchArg quiet_arg("q", "quiet", "Disables logging to console output.", cmd, false);
  TCLAP::SwitchArg save_log_arg("s", "save_log", "Enables log file creation in a folder called \"log\" next to the executable.", cmd, false);
  TCLAP::SwitchArg one_file_per_topic_arg("", "enable-one-file-per-topic", "Whether to separate each topic in single HDF5 file.", cmd, false);

  try
  {
    cmd.parse(arguments);
  }
  catch (TCLAP::ArgException& e)  // catch any exceptions
  {
    eCALMeasCutterUtils::printError(e.error());
    std::exit(EXIT_FAILURE);
  }

  //////////////////////////////////////////////////////////////////////////////
  //// Argument constraints                                                 ////
  //////////////////////////////////////////////////////////////////////////////

  _config_file_path = config_file_path_arg.getValue();

  eCALMeasCutterUtils::quiet                     = quiet_arg.getValue();
  eCALMeasCutterUtils::save_log                  = save_log_arg.getValue();
  eCALMeasCutterUtils::enable_one_file_per_topic = one_file_per_topic_arg.getValue();

  if (eCALMeasCutterUtils::save_log)
  {
    eCALMeasCutterUtils::createLogFile();
  }

  auto input_measurement_list = input_measurement_arg.getValue();
  auto output_measurement_list = output_measurement_arg.getValue();

  if (input_measurement_list.size() != output_measurement_list.size())
  {
    eCALMeasCutterUtils::printError("One input measurement has to have one output directory!");
    eCALMeasCutterUtils::printError("Tip: A more extensive help is available by --help.");

    std::exit(EXIT_FAILURE);
  }

  // clean paths before comparing them for duplicated
  for (auto &path : input_measurement_list)
  {
    path = EcalUtils::Filesystem::ToNativeSeperators(EcalUtils::Filesystem::CleanPath(path, EcalUtils::Filesystem::OsStyle::Current));
  }
  for (auto &path : output_measurement_list)
  {
    path = EcalUtils::Filesystem::ToNativeSeperators(EcalUtils::Filesystem::CleanPath(path, EcalUtils::Filesystem::OsStyle::Current));
  }
    
  if (hasDuplicates(input_measurement_list) || hasDuplicates(output_measurement_list))
  {
    eCALMeasCutterUtils::printError("Duplicated input/output arguments were found!");
    std::exit(EXIT_FAILURE);
  }

  for (std::vector<std::string>::size_type i = 0; i < input_measurement_list.size(); i++)
  {
    _input_output_pairs.emplace_back(std::make_pair(input_measurement_list[i], output_measurement_list[i]));
  }

  //////////////////////////////////////////////////////////////////////////////
  //// Parse config file                                                    ////
 //////////////////////////////////////////////////////////////////////////////

  parseConfigFile();
}
 
void eCALMeasCutter::parseConfigFile()
{
  ConfigFileParser parser(_config_file_path);

  if (!parser.isConfigFileValid())
    std::exit(EXIT_FAILURE);

  parser.parseTrimOperations(_trim_operation);
  parser.parseChannelsOperations(_include_list, _include_list_regex, eCALMeasCutterUtils::ChannelOperationType::include);
  parser.parseChannelsOperations(_exclude_list, _exclude_list_regex, eCALMeasCutterUtils::ChannelOperationType::exclude);
  parser.parseConversionParameters(_base_name, _max_size_per_file, _is_base_name_mention_in_config, _is_max_file_size_mention_in_config);

  auto is_include_present = false;
  auto is_exclude_present = false;
  if (!_include_list.empty() || !_include_list_regex.empty())
  {
    is_include_present = true;
  }

  if (!_exclude_list.empty() || !_exclude_list_regex.empty())
  {
    is_exclude_present = true;
  }

  // if both include and exclude lists are present, no channels manipulation will take place
  if (is_include_present && is_exclude_present)
  {
    eCALMeasCutterUtils::printError("No channels will be manipulated : Specify only the channels to be included/excluded, not both.");
    _include_list.clear();
    _exclude_list.clear();
    _include_list_regex.clear();
    _exclude_list_regex.clear();
  }

  // Check general rules for trimming operations
  checkTrimmingOperationGeneralRules();

  // check if we have something to work on
  // either trimming start or end
  // either channels to manipulate
  // or both
  if (!(_trim_operation._is_start_set) && !(_trim_operation._is_end_set) && _include_list.empty() && _exclude_list.empty() && _include_list_regex.empty() && _exclude_list_regex.empty()
        && !_is_base_name_mention_in_config && !_is_max_file_size_mention_in_config)
  {
    eCALMeasCutterUtils::printError("Failed loading configuration file from \"" + _config_file_path + "\": Nothing to do.");
    std::exit(EXIT_FAILURE);
  }
}

bool eCALMeasCutter::hasDuplicates(const std::vector<std::string>& vector) 
{
  for (auto it = vector.begin(); it != vector.end(); it++)
  {
    if (std::find(it + 1, vector.end(), *it) != vector.end())
    {
      return true;
    }
  }
  return false;
}

void eCALMeasCutter::checkTrimmingOperationGeneralRules()
{
  // is no trimming is desired, do nothing
  if (!_trim_operation._is_start_trimming_specified && !_trim_operation._is_end_trimming_specified)
    return;

  // check if trimming the start is specified
  if (_trim_operation._is_start_trimming_specified)
  {
    // check if all needed parameters are set for trimming start
    if (_trim_operation._start_time != std::numeric_limits<double>::min() && _trim_operation._start_time > 0
         && _trim_operation._is_start_scale_valid &&
        _trim_operation._start_base_type != eCALMeasCutterUtils::BaseType::null)
    {
      _trim_operation._is_start_set = true;
    }
    else
    {
      _trim_operation._is_valid = false;
      if (_trim_operation._start_time == std::numeric_limits<double>::min())
      {
        eCALMeasCutterUtils::printError("No valid start time was set to trim.");
      }
      if (_trim_operation._start_time < 0)
      {
        eCALMeasCutterUtils::printError("Negative start time was set to trim.");
      }
      if (!_trim_operation._is_start_scale_valid)
      {
        eCALMeasCutterUtils::printError("No valid start scale was set to trim.");
      }
      if (_trim_operation._start_base_type == eCALMeasCutterUtils::BaseType::null)
      {
        eCALMeasCutterUtils::printError("No valid start base type was set to trim.");
      }
    }
  }

  // check if trimming the end is specified
  if (_trim_operation._is_end_trimming_specified)
  {
    // check if all needed parameters are set for trimming end
    if (_trim_operation._end_time != std::numeric_limits<double>::max() && _trim_operation._end_time > 0
         && _trim_operation._is_end_scale_valid &&
        _trim_operation._end_base_type != eCALMeasCutterUtils::BaseType::null)
    {
      _trim_operation._is_end_set = true;
    }
    else
    {
      _trim_operation._is_valid = false;
      if (_trim_operation._end_time == std::numeric_limits<double>::max())
      {
        eCALMeasCutterUtils::printError("No valid end time was set to trim.");
      }
      if (_trim_operation._end_time < 0)
      {
        eCALMeasCutterUtils::printError("Negative end time was set to trim.");
      }
      if (!_trim_operation._is_end_scale_valid)
      {
        eCALMeasCutterUtils::printError("No valid end scale was set to trim.");
      }
      if (_trim_operation._end_base_type == eCALMeasCutterUtils::BaseType::null)
      {
        eCALMeasCutterUtils::printError("No valid end base type was set to trim.");
      }
    }
  }
}

int eCALMeasCutter::run()
{
  auto id = 0;

  _thread_pool.Start();

  for (const auto& input_output_pair : _input_output_pairs)
  {
    id++;
    eCALMeasCutterUtils::MeasurementJob  measurement_job;
    measurement_job.id = "measurement_" + std::to_string(id);
    measurement_job.trim_operation = _trim_operation;

    if (!_include_list.empty() || !_include_list_regex.empty())
    {
      measurement_job.operation_type = eCALMeasCutterUtils::ChannelOperationType::include;
      measurement_job.channels_to_manipulate = _include_list;
      measurement_job.channels_to_manipulate_regex = _include_list_regex;
    }
    else if(!_exclude_list.empty() || !_exclude_list_regex.empty())
    {
      measurement_job.operation_type = eCALMeasCutterUtils::ChannelOperationType::exclude;
      measurement_job.channels_to_manipulate = _exclude_list;
      measurement_job.channels_to_manipulate_regex = _exclude_list_regex;
    }

    measurement_job.input_measurement_path  = input_output_pair.first;
    measurement_job.output_measurement_path = input_output_pair.second;

    measurement_job.basename_mentioned_in_config = _is_base_name_mention_in_config;
    if (_is_base_name_mention_in_config)
      measurement_job.basename = _base_name;
     
    measurement_job.max_size_mentioned_in_config = _is_max_file_size_mention_in_config;
    if(_is_max_file_size_mention_in_config)
      measurement_job.max_size_per_file = _max_size_per_file;

    _thread_pool.Enque(measurement_job);
  }

  _thread_pool.processJobs();

  Logger::close();
  return 0;
}
