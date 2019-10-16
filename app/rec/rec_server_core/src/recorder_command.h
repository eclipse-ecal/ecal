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

#include <rec_core/job_config.h>

namespace eCAL
{
  namespace rec
  {
    struct RecorderCommand
    {
      enum class Type
      {
        NONE,
        INITIALIZE,
        DE_INITIALIZE,
        START_RECORDING,
        STOP_RECORDING,
        SAVE_PRE_BUFFER,
        ADD_SCENARIO,
        EXIT,
      };

      RecorderCommand()
        : type_(Type::NONE)
      {}

      Type        type_;
      JobConfig   job_config_;
      std::string scenario_name_;
    };
  }
}