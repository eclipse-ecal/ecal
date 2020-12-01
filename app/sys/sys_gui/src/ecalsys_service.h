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

/**
 * eCALSys Service
**/

#pragma once

#include "ecalsys_gui.h"

// protobuf remote control
#include "ecal/pb/sys/service.pb.h"

class eCALSysServiceImpl : public eCAL::pb::sys::Service
{
public:
  eCALSysServiceImpl(EcalsysGui* inst) : ecalsysgui_instance(inst) {};

  void StartTasks(::google::protobuf::RpcController* controller,
    const ::eCAL::pb::sys::TaskRequest* request,
    ::eCAL::pb::sys::Response* response,
    ::google::protobuf::Closure* done);
  void StopTasks(::google::protobuf::RpcController* controller,
    const ::eCAL::pb::sys::TaskRequest* request,
    ::eCAL::pb::sys::Response* response,
    ::google::protobuf::Closure* done);
  void RestartTasks(::google::protobuf::RpcController* controller,
    const ::eCAL::pb::sys::TaskRequest* request,
    ::eCAL::pb::sys::Response* response,
    ::google::protobuf::Closure* done);
  void GetStatus(::google::protobuf::RpcController* controller,
    const ::eCAL::pb::sys::GenericRequest* request,
    ::eCAL::pb::sys::State* response,
    ::google::protobuf::Closure* done) override;

private:
  EcalsysGui* ecalsysgui_instance;
};
