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
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/pb/rec/server_service.pb.h>
#include <ecal/pb/rec/server_state.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class RecServerService : public eCAL::pb::rec_server::EcalRecServerService
{
public:
  RecServerService();
  ~RecServerService();

  virtual void LoadConfigFile(::google::protobuf::RpcController*               controller
                            , const ::eCAL::pb::rec_server::LoadConfigRequest* request
                            , ::eCAL::pb::rec_server::ServiceResult*           response
                            , ::google::protobuf::Closure*                     done) override;

  virtual void Activate(::google::protobuf::RpcController*                  controller
                            , const ::eCAL::pb::rec_server::GenericRequest* request
                            , ::eCAL::pb::rec_server::ServiceResult*        response
                            , ::google::protobuf::Closure*                  done) override;

  virtual void DeActivate(::google::protobuf::RpcController*                controller
                            , const ::eCAL::pb::rec_server::GenericRequest* request
                            , ::eCAL::pb::rec_server::ServiceResult*        response
                            , ::google::protobuf::Closure*                  done) override;

  virtual void StartRecording(::google::protobuf::RpcController*            controller
                            , const ::eCAL::pb::rec_server::GenericRequest* request
                            , ::eCAL::pb::rec_server::JobStartedResponse*   response
                            , ::google::protobuf::Closure*                  done) override;

  virtual void StopRecording(::google::protobuf::RpcController*             controller
                            , const ::eCAL::pb::rec_server::GenericRequest* request
                            , ::eCAL::pb::rec_server::ServiceResult*        response
                            , ::google::protobuf::Closure*                  done) override;

  virtual void SaveBuffer(::google::protobuf::RpcController*                controller
                            , const ::eCAL::pb::rec_server::GenericRequest* request
                            , ::eCAL::pb::rec_server::JobStartedResponse*   response
                            , ::google::protobuf::Closure*                  done) override;

  virtual void UploadMeasurement(::google::protobuf::RpcController*                    controller
                            , const ::eCAL::pb::rec_server::GenericMeasurementRequest* request
                            , ::eCAL::pb::rec_server::ServiceResult*                   response
                            , ::google::protobuf::Closure*                             done) override;

  virtual void AddComment(::google::protobuf::RpcController*                   controller
                            , const ::eCAL::pb::rec_server::AddCommentRequest* request
                            , ::eCAL::pb::rec_server::ServiceResult*           response
                            , ::google::protobuf::Closure*                     done) override;

  virtual void DeleteMeasurement(::google::protobuf::RpcController*                    controller
                            , const ::eCAL::pb::rec_server::GenericMeasurementRequest* request
                            , ::eCAL::pb::rec_server::ServiceResult*                   response
                            , ::google::protobuf::Closure*                             done) override;

  virtual void GetStatus(::google::protobuf::RpcController*                 controller
                            , const ::eCAL::pb::rec_server::GenericRequest* request
                            , ::eCAL::pb::rec_server::Status*               response
                            , ::google::protobuf::Closure*                  done) override;
};

