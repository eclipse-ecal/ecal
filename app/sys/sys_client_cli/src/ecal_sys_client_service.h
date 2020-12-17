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

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4127 4505 4800)
#endif
#include <ecal/pb/sys/client_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class EcalSysClientService : public eCAL::pb::sys_client::EcalSysClientService
{
public:
  EcalSysClientService();
  ~EcalSysClientService();

  virtual void StartTasks(::google::protobuf::RpcController*              controller
                        , const ::eCAL::pb::sys_client::StartTaskRequest* request
                        , ::eCAL::pb::sys_client::TaskResponse*           response
                        , ::google::protobuf::Closure*                    done) override;

  virtual void StopTasks(::google::protobuf::RpcController*             controller
                       , const ::eCAL::pb::sys_client::StopTaskRequest* request
                       , ::eCAL::pb::sys_client::TaskResponse*          response
                       , ::google::protobuf::Closure*                   done) override;

  virtual void MatchTasks(::google::protobuf::RpcController*            controller
                       , const ::eCAL::pb::sys_client::TaskList*        request
                       , ::eCAL::pb::sys_client::MatchTaskResponse*     response
                       , ::google::protobuf::Closure*                   done) override;
};

