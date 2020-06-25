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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800) // disable proto warnings
#endif
#include <ecal/pb/rec/server_state.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  namespace rec_server
  {
    namespace proto_helpers
    {
      void ToProtobuf(const eCAL::rec_server::ClientJobStatus& client_job_status, eCAL::pb::rec_server::ClientJobStatus& client_job_status_pb);
      void ToProtobuf(const eCAL::rec_server::JobHistoryEntry& job_history_entry, eCAL::pb::rec_server::Measurement& measurement_pb);
      void ToProtobuf(const eCAL::rec_server::RecServerStatus& rec_server_status, eCAL::pb::rec_server::Status& rec_server_status_pb);

      eCAL::pb::rec_server::ClientJobStatus ToProtobuf(const eCAL::rec_server::ClientJobStatus& client_job_status);
      eCAL::pb::rec_server::Measurement ToProtobuf(const eCAL::rec_server::JobHistoryEntry& job_history_entry);
      eCAL::pb::rec_server::Status ToProtobuf(const eCAL::rec_server::RecServerStatus& rec_server_status);

    }
  }
}
