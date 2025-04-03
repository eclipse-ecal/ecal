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

#include "ecal_sys_client_service.h"

#include <sys_client_core/proto_helpers.h>
#include <sys_client_core/ecal_sys_client.h>

#include<ecal/process.h>

EcalSysClientService::EcalSysClientService()
{}

EcalSysClientService::~EcalSysClientService()
{}

void EcalSysClientService::StartTasks(::google::protobuf::RpcController*              /*controller*/
                                   , const ::eCAL::pb::sys_client::StartTaskRequest* request
                                   , ::eCAL::pb::sys_client::TaskResponse*           response
                                   , ::google::protobuf::Closure*                    /*done*/)
{
  response->Clear();

  std::vector<int32_t> process_ids = eCAL::sys_client::StartTasks(eCAL::sys_client::proto_helpers::FromProtobuf(*request));

  for (int32_t process_id : process_ids)
  {
    auto task_response = response->mutable_responses()->Add();
    task_response->set_process_id(process_id);
    task_response->set_result(process_id != 0 ? eCAL::pb::sys_client::eServiceResult::success : eCAL::pb::sys_client::eServiceResult::failed);
    task_response->set_error (process_id != 0 ? "" : "Error starting task");
  }
}

void EcalSysClientService::StopTasks(::google::protobuf::RpcController*             /*controller*/
                                  , const ::eCAL::pb::sys_client::StopTaskRequest* request
                                  , ::eCAL::pb::sys_client::TaskResponse*          response
                                  , ::google::protobuf::Closure*                   /*done*/)
{
  response->Clear();

  std::vector<bool> success_list = eCAL::sys_client::StopTasks(eCAL::sys_client::proto_helpers::FromProtobuf(*request));

  for (bool success: success_list)
  {
    auto task_response = response->mutable_responses()->Add();
    task_response->set_result(success ? eCAL::pb::sys_client::eServiceResult::success : eCAL::pb::sys_client::eServiceResult::failed);
    task_response->set_error (success ? "" : "Error stopping task");
  }
}

void EcalSysClientService::MatchTasks(::google::protobuf::RpcController*        /*controller*/
                                   , const ::eCAL::pb::sys_client::TaskList*    request
                                   , ::eCAL::pb::sys_client::MatchTaskResponse* response
                                   , ::google::protobuf::Closure*               /*done*/)
{
  std::vector<eCAL::sys_client::Task> task_list = eCAL::sys_client::proto_helpers::FromProtobuf(*request);
  std::vector<std::vector<int32_t>> list_of_pid_lists = eCAL::sys_client::MatchTasks(task_list);  
  eCAL::sys_client::proto_helpers::ToProtobuf(*response, list_of_pid_lists);
}
