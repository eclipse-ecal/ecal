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
#pragma warning(disable : 4100 4127 4505 4800)
#endif
#include <ecal/pb/rec/client_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <atomic>

#include <rec_client_core/state.h>
#include <rec_client_core/job_config.h>
#include <rec_client_core/upload_config.h>

namespace eCAL
{
  namespace rec
  {
    class EcalRec;
  }
}

class EcalRecService : public eCAL::pb::rec_client::EcalRecClientService
{
public:
  EcalRecService(std::shared_ptr<eCAL::rec::EcalRec> ecal_rec);
  ~EcalRecService();

  virtual void GetConfig(::google::protobuf::RpcController*        controller
                        , const ::eCAL::pb::rec_client::GetConfigRequest* request
                        , ::eCAL::pb::rec_client::GetConfigResponse*      response
                        , ::google::protobuf::Closure*             done);

  virtual void SetConfig(::google::protobuf::RpcController*        controller
                        , const ::eCAL::pb::rec_client::SetConfigRequest* request
                        , ::eCAL::pb::rec_client::Response*               response
                        , ::google::protobuf::Closure*             done);

  virtual void SetCommand(::google::protobuf::RpcController*       controller
                        , const ::eCAL::pb::rec_client::CommandRequest*   request
                        , ::eCAL::pb::rec_client::Response*               response
                        , ::google::protobuf::Closure*             done);

  virtual void GetState(::google::protobuf::RpcController*         controller
                        , const ::eCAL::pb::rec_client::GetStateRequest*  request
                        , ::eCAL::pb::rec_client::State*                  response
                        , ::google::protobuf::Closure*             done);

  bool IsExitCommandSet() const;

private:
  std::shared_ptr<eCAL::rec::EcalRec> ecal_rec_;
  std::atomic<bool> exit_command_;

  static bool strToBool(const std::string& str);

  static ::eCAL::rec::JobConfig    ToJobConfig  (const ::eCAL::pb::rec_client::Configuration& config, ::eCAL::pb::rec_client::Response* response);
  static ::eCAL::rec::UploadConfig ToUploadConfig(const ::eCAL::pb::rec_client::Configuration& config, ::eCAL::pb::rec_client::Response* response);

};

