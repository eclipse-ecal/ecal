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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800) // disable proto warnings
#endif
#include <ecal/pb/play/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <atomic>

class EcalPlay;

class EcalPlayService : public eCAL::pb::play::EcalPlayService
{
public:
  EcalPlayService(std::shared_ptr<EcalPlay> ecal_player);
  ~EcalPlayService();

  virtual void GetConfig(::google::protobuf::RpcController*         controller
                        , const ::eCAL::pb::play::GetConfigRequest* request
                        , ::eCAL::pb::play::GetConfigResponse*      response
                        , ::google::protobuf::Closure*              done);

  virtual void SetConfig(::google::protobuf::RpcController*         controller
                        , const ::eCAL::pb::play::SetConfigRequest* request
                        , ::eCAL::pb::play::Response*               response
                        , ::google::protobuf::Closure*              done);

  virtual void SetCommand(::google::protobuf::RpcController*        controller
                        , const ::eCAL::pb::play::CommandRequest*   request
                        , ::eCAL::pb::play::Response*               response
                        , ::google::protobuf::Closure*              done);

  bool IsExitRequested() const;

private:
  std::shared_ptr<EcalPlay> ecal_player_;
  std::atomic<bool> exit_request_;
  static bool strToBool(const std::string& str);
};

