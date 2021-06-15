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

#include "status.h"
#include <rec_client_core/rec_error.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/pb/rec/server_state.pb.h>
#include <ecal/pb/rec/server_service.pb.h>
#include <ecal/pb/rec/server_config.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  namespace rec_server
  {
    namespace proto_helpers
    {
      ///////////////////////////////
      /// To Protobuf
      ///////////////////////////////
      
      void ToProtobuf(const eCAL::rec_server::UploadConfig&    upload_config,     eCAL::pb::rec_server::UploadConfig&    upload_config_pb);
      void ToProtobuf(const eCAL::rec_server::ClientConfig&    rec_client_config, eCAL::pb::rec_server::RecClientConfig& rec_client_config_pb);
      void ToProtobuf(const eCAL::rec::RecordMode&             record_mode,       eCAL::pb::rec_server::RecordMode&      record_mode_pb);
      void ToProtobuf(const eCAL::rec_server::RecServerConfig& rec_server_config, eCAL::pb::rec_server::RecServerConfig& rec_server_config_pb);
      void ToProtobuf(const eCAL::rec_server::ClientJobStatus& client_job_status, eCAL::pb::rec_server::ClientJobStatus& client_job_status_pb);
      void ToProtobuf(const eCAL::rec_server::JobHistoryEntry& job_history_entry, eCAL::pb::rec_server::Measurement&     measurement_pb);
      void ToProtobuf(const std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point>& client_status_pair, const std::string& hostname, eCAL::pb::rec_server::ClientStatus& client_status_pb);
      void ToProtobuf(const eCAL::rec_server::RecServerStatus& rec_server_status, eCAL::pb::rec_server::Status&          rec_server_status_pb);

      eCAL::pb::rec_server::UploadConfig    ToProtobuf(const eCAL::rec_server::UploadConfig&    upload_config);
      eCAL::pb::rec_server::RecClientConfig ToProtobuf(const eCAL::rec_server::ClientConfig&    rec_client_config);
      eCAL::pb::rec_server::RecordMode      ToProtobuf(const eCAL::rec::RecordMode&             record_mode);
      eCAL::pb::rec_server::RecServerConfig ToProtobuf(const eCAL::rec_server::RecServerConfig& rec_server_config);
      eCAL::pb::rec_server::ClientJobStatus ToProtobuf(const eCAL::rec_server::ClientJobStatus& client_job_status);
      eCAL::pb::rec_server::Measurement     ToProtobuf(const eCAL::rec_server::JobHistoryEntry& job_history_entry);
      eCAL::pb::rec_server::ClientStatus    ToProtobuf(const std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point>& client_status_pair, const std::string& hostname);
      eCAL::pb::rec_server::Status          ToProtobuf(const eCAL::rec_server::RecServerStatus& rec_server_status);

      ///////////////////////////////
      /// From Protobuf
      ///////////////////////////////

      void FromProtobuf(const eCAL::pb::rec_server::UploadConfig& upload_config_pb,        eCAL::rec_server::UploadConfig& upload_config);
      void FromProtobuf(const eCAL::pb::rec_server::RecClientConfig& rec_client_config_pb, eCAL::rec_server::ClientConfig&    rec_client_config);
      void FromProtobuf(const eCAL::pb::rec_server::RecordMode&      record_mode_pb,       eCAL::rec::RecordMode&             record_mode);
      void FromProtobuf(const eCAL::pb::rec_server::RecServerConfig& rec_server_config_pb, eCAL::rec_server::RecServerConfig& rec_server_config);
      void FromProtobuf(const eCAL::pb::rec_server::ClientJobStatus& client_job_status_pb, eCAL::rec_server::ClientJobStatus& client_job_status);
      void FromProtobuf(const eCAL::pb::rec_server::Measurement& measurement_pb,           eCAL::rec_server::JobHistoryEntry& job_history_entry);
      void FromProtobuf(const eCAL::pb::rec_server::ClientStatus& client_status_pb,        std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point>& client_status_pair, std::string& hostname);
      void FromProtobuf(const eCAL::pb::rec_server::Status& rec_server_status_pb,          eCAL::rec_server::RecServerStatus& rec_server_status);

      eCAL::rec_server::UploadConfig    FromProtobuf(const eCAL::pb::rec_server::UploadConfig&    upload_config_pb);
      eCAL::rec_server::ClientConfig    FromProtobuf(const eCAL::pb::rec_server::RecClientConfig& rec_client_config_pb);
      eCAL::rec::RecordMode             FromProtobuf(const eCAL::pb::rec_server::RecordMode&      record_mode_pb);
      eCAL::rec_server::RecServerConfig FromProtobuf(const eCAL::pb::rec_server::RecServerConfig& rec_server_config_pb);
      eCAL::rec_server::ClientJobStatus FromProtobuf(const eCAL::pb::rec_server::ClientJobStatus& client_job_status_pb);
      eCAL::rec_server::JobHistoryEntry FromProtobuf(const eCAL::pb::rec_server::Measurement&     measurement_pb);
      std::pair<std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point>, std::string> FromProtobuf(const eCAL::pb::rec_server::ClientStatus& client_status_pb);
      eCAL::rec_server::RecServerStatus FromProtobuf(const eCAL::pb::rec_server::Status&          rec_server_status_pb);

      ///////////////////////////////
      /// Service From Protobuf
      ///////////////////////////////
      
      void FromProtobuf(const eCAL::pb::rec_server::ServiceResult& service_result_pb, eCAL::rec::Error& error);

      eCAL::rec::Error FromProtobuf(const eCAL::pb::rec_server::ServiceResult& service_result_pb);
    }
  }
}
