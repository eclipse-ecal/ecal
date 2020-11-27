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

#include <vector>
#include <string>
#include <sys_error.h>

#include <memory>
#include <ecalsys/ecal_sys.h>

namespace eCAL
{
  namespace sys
  {
    namespace command
    {
      Error ToTaskList(const std::shared_ptr<EcalSys> ecalsys_instance, const std::vector<std::string>& argv, std::list<std::shared_ptr<EcalSysTask>>& output_tasklist)
      {
        output_tasklist.clear();

        if (argv.empty())
        {
          output_tasklist = ecalsys_instance->GetTaskList();
        }
        else
        {
          auto complete_task_list = ecalsys_instance->GetTaskList();

          for (const std::string& arg : argv)
          {
            bool match_found = false;

            // Try matching the argument as task ID
            try
            {
              unsigned long id = std::stoul(arg);
              auto task_ptr = ecalsys_instance->GetTask(static_cast<uint32_t>(id));
              if (task_ptr)
              {
                output_tasklist.push_back(task_ptr);
                match_found = true;
              }
            }
            catch (const std::exception&) {}

            // Try matching the argument as task Name
            if (!match_found)
            {
              for (const auto& task : complete_task_list)
              {
                if (task->GetName() == arg)
                {
                  output_tasklist.push_back(task);
                  match_found = true; 
                  break;
                }
              }
            }

            // Return error if unable to find task
            if (!match_found)
            {
              return eCAL::sys::Error(eCAL::sys::Error::ErrorCode::TASK_DOES_NOT_EXIST, arg);
            }
          }
        }

        return eCAL::sys::Error::ErrorCode::OK;
      }
    }
  }
}