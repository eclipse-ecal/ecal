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

#include "start_tasks.h"
#include "helpers.h"

namespace eCAL
{
  namespace sys
  {
    namespace command
    {

      std::string StartTask::Usage() const
      {
        return "[IDs or names]";
      }

      std::string StartTask::Help() const
      {
        return "Start tasks with the given IDs or names. If no ID or name is given, all tasks will be started.";
      }

      std::string StartTask::Example() const
      {
        return "name1 name2 \"name with spaces\" 12345";
      }

      eCAL::sys::Error StartTask::Execute(const std::shared_ptr<EcalSys>& ecalsys_instance, const std::vector<std::string>& argv)
      {
        std::list<std::shared_ptr<EcalSysTask>> task_list;
        auto error = ToTaskList(ecalsys_instance, argv, task_list);

        if (error)
        {
          return error;
        }
        else
        {
          ecalsys_instance->StartTaskList(task_list);
          return eCAL::sys::Error::ErrorCode::OK;
        }
      }
    }
  }
}