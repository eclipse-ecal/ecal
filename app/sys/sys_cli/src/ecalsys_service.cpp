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

/**
 * eCALSys Service
**/

#include "ecalsys/ecal_sys.h"
#include "ecalsys/ecal_sys_logger.h"
#include "ecalsys_service.h"

#include <ecalsys/proto_helpers.h>

extern bool exit_command_received;

namespace
{
  std::list<std::shared_ptr<EcalSysTask>> GetAllTasks(std::shared_ptr<EcalSys> ecalsys_instance)
  {
    auto tasks   = ecalsys_instance->GetTaskList();
    auto options = ecalsys_instance->GetOptions();

    if (options.local_tasks_only)
    {
      // Remove all non-local tasks
      tasks.remove_if(
        [](std::shared_ptr<EcalSysTask> task)
      {
        return task->GetTarget() != eCAL::Process::GetHostName();
      });
    }
    return tasks;
  }

  std::list<std::shared_ptr<EcalSysTask>> GetRequestedTasks(std::shared_ptr<EcalSys> ecalsys_instance, const ::eCAL::pb::sys::TaskRequest* request)
  {
    std::set<std::shared_ptr<EcalSysTask>> tasks;
    for (int32_t id : request->tids())
    {
      tasks.emplace(ecalsys_instance->GetTask(id));
    }
    for (std::string task_name : request->tnames())
    {
      for (auto& task : ecalsys_instance->GetTaskList())
      {
        if (task->GetName() == task_name)
        {
          tasks.emplace(task);
        }
      }
    }
    return std::list<std::shared_ptr<EcalSysTask>>(tasks.begin(), tasks.end());
  }
}

void eCALSysServiceImpl::StartTasks(::google::protobuf::RpcController* /*controller*/,
  const ::eCAL::pb::sys::TaskRequest* request,
  ::eCAL::pb::sys::Response* response,
  ::google::protobuf::Closure* /*done*/)
{
  bool all_tasks(request->all());
  if (all_tasks)
  {
    EcalSysLogger::Log("eCALSysServiceImpl::StartTasks - start all tasks");
    ecalsys_instance->StartTaskList(GetAllTasks(ecalsys_instance));
  }
  else
  {
    EcalSysLogger::Log("eCALSysServiceImpl::StartTasks - start list of tasks");
    ecalsys_instance->StartTaskList(GetRequestedTasks(ecalsys_instance, request));
  }

  response->set_result(eCAL::pb::sys::Response::success);
}

void eCALSysServiceImpl::StopTasks(::google::protobuf::RpcController* /*controller*/,
  const ::eCAL::pb::sys::TaskRequest* request,
  ::eCAL::pb::sys::Response* response,
  ::google::protobuf::Closure* /*done*/)
{
  bool all_tasks(request->all());
  if (all_tasks)
  {
    EcalSysLogger::Log("eCALSysServiceImpl::StopTasks - stop all tasks");
    ecalsys_instance->StopTaskList(GetAllTasks(ecalsys_instance), true, true);
  }
  else
  {
    EcalSysLogger::Log("eCALSysServiceImpl::StopTasks - stop list of tasks");
    ecalsys_instance->StopTaskList(GetRequestedTasks(ecalsys_instance, request), true, true);
  }

  response->set_result(eCAL::pb::sys::Response::success);
}

void eCALSysServiceImpl::RestartTasks(::google::protobuf::RpcController* /*controller*/,
  const ::eCAL::pb::sys::TaskRequest* request,
  ::eCAL::pb::sys::Response* response,
  ::google::protobuf::Closure* /*done*/)
{
  bool all_tasks(request->all());
  if (all_tasks)
  {
    EcalSysLogger::Log("eCALSysServiceImpl::RestartTasks - restart all tasks");
    ecalsys_instance->RestartTaskList(GetAllTasks(ecalsys_instance), true, true);
  }
  else
  {
    EcalSysLogger::Log("eCALSysServiceImpl::RestartTasks - restart list of tasks");
    ecalsys_instance->RestartTaskList(GetRequestedTasks(ecalsys_instance, request), true, true);
  }

  response->set_result(eCAL::pb::sys::Response::success);
}

void eCALSysServiceImpl::GetStatus(::google::protobuf::RpcController* /*controller*/,
  const ::eCAL::pb::sys::GenericRequest* /*request*/,
  ::eCAL::pb::sys::State* response,
  ::google::protobuf::Closure* /*done*/)
{
  eCAL::sys::proto_helpers::ToProtobuf(*response, *ecalsys_instance.get());
}
