/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <ecal/process_mode.h>
#include <ecal_utils/filesystem.h>
#include <chrono>

#include "runner.h"

namespace eCAL
{
  namespace sys_client
  {
    struct Task
    {
      std::string path;
      std::string arguments;
      std::string working_dir;
      Runner runner;
    };

    struct StartTaskParameters
    {
      StartTaskParameters()
        : window_mode   (eCAL::Process::eStartMode::normal)
        , create_console(false)
      {}

      Task                    task;
      eCAL::Process::eStartMode window_mode;
      bool                    create_console;
    };

    struct StopTaskParameters
    {
      StopTaskParameters()
        : process_id          (0)
        , ecal_shutdown(false)
      {}

      int32_t process_id;
      Task    task;
      bool    ecal_shutdown;
    };

    void EvaluateEcalParserFunctions(eCAL::sys_client::Task& task, bool eval_target_functions, std::chrono::system_clock::time_point now);

    void MergeRunnerIntoTask(eCAL::sys_client::Task& task, EcalUtils::Filesystem::OsStyle input_path_style);
  }
}