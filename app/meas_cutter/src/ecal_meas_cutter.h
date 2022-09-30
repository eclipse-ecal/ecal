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

#include <limits>

#include "tclap/CmdLine.h"
#include <ecal_utils/command_line.h>

#include "config_file_parser.h"
#include "ecal_meas_cutter_globals.h"
#include "thread_pool.h"


#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif


class eCALMeasCutter
{
public:
  eCALMeasCutter(std::vector<std::string>& arguments);
  int run();

private:
  void                                             parseCmdLine(std::vector<std::string>& arguments);
  void                                             parseConfigFile();
  bool                                             hasDuplicates(const std::vector<std::string>& vector);
  void                                             checkTrimmingOperationGeneralRules();

  std::string                                      _config_file_path;
  std::list<std::pair<std::string, std::string>>   _input_output_pairs;
  eCALMeasCutterUtils::TrimOperation               _trim_operation;
  std::list<std::string>                           _include_list;
  std::list<std::string>                           _exclude_list;
  std::list<std::string>                           _include_list_regex;
  std::list<std::string>                           _exclude_list_regex;

  std::string                                      _base_name;
  size_t                                           _max_size_per_file;
  bool                                             _is_base_name_mention_in_config;
  bool                                             _is_max_file_size_mention_in_config;

  ThreadPool                                       _thread_pool;
};

