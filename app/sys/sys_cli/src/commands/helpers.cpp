/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "helpers.h"

#include <vector>
#include <string>
#include <sys_error.h>

#include <memory>
#include <ecalsys/ecal_sys.h>

#include <ecalsys/proto_helpers.h>

namespace eCAL
{
  namespace sys
  {
    namespace command
    {
      Error ToTaskList(const std::shared_ptr<EcalSys> ecalsys_instance, const std::vector<std::string>& argv, std::list<std::shared_ptr<EcalSysTask>>& output_tasklist)
      {
        if (argv.empty())
        {
          output_tasklist = ecalsys_instance->GetTaskList();
        }
        else
        {
          output_tasklist.clear();
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


      Error ToRunnerList(const std::shared_ptr<EcalSys> ecalsys_instance, const std::vector<std::string>& argv, std::list<std::shared_ptr<EcalSysRunner>>& output_runnerlist)
      {
        if (argv.empty())
        {
          output_runnerlist = ecalsys_instance->GetRunnerList();
        }
        else
        {
          output_runnerlist.clear();
          auto complete_runner_list = ecalsys_instance->GetRunnerList();

          for (const std::string& arg : argv)
          {
            bool match_found = false;

            // Try matching the argument as runner ID
            try
            {
              unsigned long id = std::stoul(arg);
              auto runner_ptr = ecalsys_instance->GetRunner(static_cast<uint32_t>(id));
              if (runner_ptr)
              {
                output_runnerlist.push_back(runner_ptr);
                match_found = true;
              }
            }
            catch (const std::exception&) {}

            // Try matching the argument as runner Name
            if (!match_found)
            {
              for (const auto& runner : complete_runner_list)
              {
                if (runner->GetName() == arg)
                {
                  output_runnerlist.push_back(runner);
                  match_found = true; 
                  break;
                }
              }
            }

            // Return error if unable to find runner
            if (!match_found)
            {
              return eCAL::sys::Error(eCAL::sys::Error::ErrorCode::RUNNER_DOES_NOT_EXIST, arg);
            }
          }
        }

        return eCAL::sys::Error::ErrorCode::OK;
      }

      Error ToGroupList(const std::shared_ptr<EcalSys> ecalsys_instance, const std::vector<std::string>& argv, std::list<std::shared_ptr<TaskGroup>>& output_grouplist)
      {
        if (argv.empty())
        {
          output_grouplist = ecalsys_instance->GetGroupList();
        }
        else
        {
          output_grouplist.clear();
          auto complete_group_list = ecalsys_instance->GetGroupList();

          for (const std::string& arg : argv)
          {
            bool match_found = false;

            // Try matching the argument as group ID
            try
            {
              unsigned long id = std::stoul(arg);
              for (const auto& group_ptr : complete_group_list)
              {
                if (group_ptr->GetId() == static_cast<uint32_t>(id))
                {
                  output_grouplist.push_back(group_ptr);
                  match_found = true;
                }
              }
            }
            catch (const std::exception&) {}

            // Try matching the argument as group Name
            if (!match_found)
            {
              for (const auto& group : complete_group_list)
              {
                if (group->GetName() == arg)
                {
                  output_grouplist.push_back(group);
                  match_found = true; 
                  break;
                }
              }
            }

            // Return error if unable to find group
            if (!match_found)
            {
              return eCAL::sys::Error(eCAL::sys::Error::ErrorCode::GROUP_DOES_NOT_EXIST, arg);
            }
          }
        }

        return eCAL::sys::Error::ErrorCode::OK;
      }

      Error GetCompleteTaskList(const eCAL::pb::sys::State& state_pb, std::list<std::shared_ptr<EcalSysTask>>& output_tasklist)
      {
        output_tasklist.clear();
        for (const auto& task_pb : state_pb.tasks())
        {
          output_tasklist.push_back(eCAL::sys::proto_helpers::FromProtobuf(task_pb));
        }
        return Error::OK;
      }

      Error GetCompleteGroupList(const eCAL::pb::sys::State& state_pb, std::list<std::shared_ptr<TaskGroup>>& output_grouplist)
      {
        output_grouplist.clear();
        for (const auto& group_pb : state_pb.groups())
        {
          output_grouplist.push_back(eCAL::sys::proto_helpers::FromProtobuf(group_pb));
        }
        return Error::OK;
      }

      Error ToTaskList(const eCAL::pb::sys::State& state_pb, const std::vector<std::string>& argv, std::list<std::shared_ptr<EcalSysTask>>& output_tasklist)
      {
        if (argv.empty())
        {
          return GetCompleteTaskList(state_pb, output_tasklist);
        }
        else
        {
          output_tasklist.clear();

          std::list<std::shared_ptr<EcalSysTask>> complete_task_list;
          GetCompleteTaskList(state_pb, complete_task_list);

          for (const std::string& arg : argv)
          {
            bool match_found = false;

            // Try matching the argument as task ID
            try
            {
              unsigned long id = std::stoul(arg);

              for (const auto& task : complete_task_list)
              {
                if (task->GetId() == id)
                {
                  output_tasklist.push_back(task);
                  match_found = true;
                  break;
                }
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

      Error ToGroupList(const eCAL::pb::sys::State& state_pb, const std::vector<std::string>& argv, std::list<std::shared_ptr<TaskGroup>>& output_grouplist)
      {
        if (argv.empty())
        {
          return GetCompleteGroupList(state_pb, output_grouplist);
        }
        else
        {
          output_grouplist.clear();

          std::list<std::shared_ptr<TaskGroup>> complete_grouplist;
          GetCompleteGroupList(state_pb, complete_grouplist);

          for (const std::string& arg : argv)
          {
            bool match_found = false;

            // Try matching the argument as group ID
            try
            {
              unsigned long id = std::stoul(arg);

              for (const auto& group : complete_grouplist)
              {
                if (group->GetId() == id)
                {
                  output_grouplist.push_back(group);
                  match_found = true;
                  break;
                }
              }
            }
            catch (const std::exception&) {}

            // Try matching the argument as group Name
            if (!match_found)
            {
              for (const auto& group : complete_grouplist)
              {
                if (group->GetName() == arg)
                {
                  output_grouplist.push_back(group);
                  match_found = true; 
                  break;
                }
              }
            }

            // Return error if unable to find group
            if (!match_found)
            {
              return eCAL::sys::Error(eCAL::sys::Error::ErrorCode::GROUP_DOES_NOT_EXIST, arg);
            }
          }

          return eCAL::sys::Error::ErrorCode::OK;
        }
      }
    }
  }
}
