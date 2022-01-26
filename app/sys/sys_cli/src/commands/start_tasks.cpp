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

      eCAL::sys::Error StartTask::Execute(const std::shared_ptr<EcalSys>& ecalsys_instance, const std::vector<std::string>& argv) const
      {
        if (argv.empty())
        {
          ecalsys_instance->StartTasks();
          return eCAL::sys::Error::ErrorCode::OK;
        }
        else
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

      eCAL::sys::Error StartTask::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::sys::Service>>& remote_ecalsys_service, const std::vector<std::string>& argv) const
      {
        SServiceResponse           service_response;
        eCAL::pb::sys::TaskRequest task_request_pb;
        eCAL::pb::sys::Response    response_pb;

        if (argv.empty())
        {
          // In this case we simply start everything
          task_request_pb.set_all(true);
        }
        else
        {
          task_request_pb.set_all(false);

          // In this case we have to check if an argument is meant to be an ID or a name
          eCAL::pb::sys::State state_pb;
          {
            auto error = GetRemoteSysStatus(hostname, remote_ecalsys_service, state_pb); 
            if (error)
              return error;
          }

          std::list<std::shared_ptr<EcalSysTask>> complete_task_list;
          {
            auto error = GetCompleteTaskList(state_pb, complete_task_list);
            if (error)
              return error;
          }

          // Iterate over argv and check if we can parse the argument to an ID. If a task with that ID exists, we assume that the ID is meant. If not, we treat it as a name.
          for (const std::string& arg : argv)
          {
            bool match_found = false;
            try
            {
              unsigned long id = std::stoul(arg);

              for (const auto& task : complete_task_list)
              {
                if (task->GetId() == id)
                {
                  task_request_pb.add_tids(id);
                  match_found = true;
                }
              }
            }
            catch (const std::exception&) {}

            if (!match_found)
            {
              task_request_pb.add_tnames(arg);
            }
          }
        }

        bool success = remote_ecalsys_service->Call(hostname, "StartTasks", task_request_pb, service_response, response_pb);

        if (!success)
        {
          return eCAL::sys::Error(eCAL::sys::Error::ErrorCode::REMOTE_HOST_UNAVAILABLE, hostname);
        }
        else
        {
          if (response_pb.result() != eCAL::pb::sys::Response::success)
          {
            return eCAL::sys::Error(eCAL::sys::Error::ErrorCode::SERVICE_CALL_RETURNED_ERROR, service_response.host_name + ": " + response_pb.error());
          }
          else
          {
            return eCAL::sys::Error::ErrorCode::OK;

          }
        }
      }

    }
  }
}