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
#include <ecal/pb/rec/client_state.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "state.h"

namespace eCAL
{
  namespace rec
  {
    namespace proto_helpers
    {
      void ToProtobuf(const eCAL::rec::RecHdf5JobStatus&        hdf5_job_status,             eCAL::pb::rec_client::State::RecHdf5Status&            hdf5_status_pb);
      void ToProtobuf(const eCAL::rec::RecAddonJobStatus&       rec_addon_job_status,        eCAL::pb::rec_client::State::RecAddonJobStatus&        rec_addon_job_status_pb);
      void ToProtobuf(const eCAL::rec::RecAddonJobStatus::State rec_addon_job_status_state , eCAL::pb::rec_client::State::RecAddonJobStatus::State& rec_addon_job_status_state_pb);
      void ToProtobuf(const eCAL::rec::UploadStatus&            upload_status,               eCAL::pb::rec_client::State::UploadStatus&             upload_status_pb);
      void ToProtobuf(const eCAL::rec::JobState                 job_state,                   eCAL::pb::rec_client::State::JobState&                 job_state_pb);
      void ToProtobuf(const eCAL::rec::JobStatus&               job_status,                  eCAL::pb::rec_client::State::JobStatus&                job_status_pb);
      void ToProtobuf(const eCAL::rec::RecorderAddonStatus&     rec_addon_status,            eCAL::pb::rec_client::State::RecorderAddonStatus&      rec_addon_status_pb);
      void ToProtobuf(const eCAL::rec::RecorderStatus& rec_status, const std::string& hostname, eCAL::pb::rec_client::State&                        rec_status_pb);

      eCAL::pb::rec_client::State::RecHdf5Status            ToProtobuf(const eCAL::rec::RecHdf5JobStatus&        hdf5_job_status);
      eCAL::pb::rec_client::State::RecAddonJobStatus        ToProtobuf(const eCAL::rec::RecAddonJobStatus&       rec_addon_job_status);
      eCAL::pb::rec_client::State::RecAddonJobStatus::State ToProtobuf(const eCAL::rec::RecAddonJobStatus::State rec_addon_job_status_state);
      eCAL::pb::rec_client::State::UploadStatus             ToProtobuf(const eCAL::rec::UploadStatus&            upload_status);
      eCAL::pb::rec_client::State::JobState                 ToProtobuf(const eCAL::rec::JobState                 job_state);
      eCAL::pb::rec_client::State::JobStatus                ToProtobuf(const eCAL::rec::JobStatus&               job_status);
      eCAL::pb::rec_client::State::RecorderAddonStatus      ToProtobuf(const eCAL::rec::RecorderAddonStatus&     rec_addon_status);
      eCAL::pb::rec_client::State                           ToProtobuf(const eCAL::rec::RecorderStatus& rec_status, const std::string& hostname);    
    }
  }
}