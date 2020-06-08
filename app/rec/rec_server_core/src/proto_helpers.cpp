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

#include <rec_server_core/proto_helpers.h>
#include <rec_client_core/proto_helpers.h>

namespace eCAL
{
  namespace rec_server
  {
    namespace proto_helpers
    {
      void ToProtobuf(const eCAL::rec_server::ClientJobStatus& client_job_status, eCAL::pb::rec_server::ClientJobStatus& client_job_status_pb)
      {
        // client_pid
        client_job_status_pb.set_client_pid(client_job_status.client_pid_);

        // job_status
        eCAL::rec::proto_helpers::ToProtobuf(client_job_status.job_status_, *client_job_status_pb.mutable_job_status());

        // last_command_response_ok
        client_job_status_pb.set_last_command_response_ok(client_job_status.info_last_command_response_.first);

        // last_command_response_message
        client_job_status_pb.set_last_command_response_message(client_job_status.info_last_command_response_.second);
      }

      void ToProtobuf(const eCAL::rec_server::JobHistoryEntry& job_history_entry, eCAL::pb::rec_server::Measurement& measurement_pb)
      {
        // id
        measurement_pb.set_id(job_history_entry.local_evaluated_job_config_.GetJobId());

        // local_timestamp_nsecs
        measurement_pb.set_local_timestamp_nsecs(std::chrono::duration_cast<std::chrono::nanoseconds>(job_history_entry.local_start_time_.time_since_epoch()).count());

        // local_name
        measurement_pb.set_local_name(job_history_entry.local_evaluated_job_config_.GetMeasName());

        // local_meas_path
        measurement_pb.set_local_meas_path(job_history_entry.local_evaluated_job_config_.GetCompleteMeasurementPath());

        // is_deleted
        measurement_pb.set_is_deleted(job_history_entry.is_deleted_);

        // client_job_statuses
        for (const auto& client_job_status : job_history_entry.client_statuses_)
        {
          (*measurement_pb.mutable_client_job_statuses())[client_job_status.first] = ToProtobuf(client_job_status.second);
        }
      }

      void ToProtobuf(const eCAL::rec_server::RecServerStatus& rec_server_status, eCAL::pb::rec_server::Status& rec_server_status_pb)
      {
        // config_path
        rec_server_status_pb.set_config_path(rec_server_status.config_path_);

        // activated
        rec_server_status_pb.set_activated(rec_server_status.activated_);

        // recording_meas_id
        rec_server_status_pb.set_recording_meas_id(rec_server_status.recording_meas_id_);

        // measurement_history
        for (const auto& history_entry : rec_server_status.job_history_)
        {
          ToProtobuf(history_entry, *rec_server_status_pb.mutable_measurement_history()->Add());
        }
      }

      eCAL::pb::rec_server::ClientJobStatus ToProtobuf(const eCAL::rec_server::ClientJobStatus& client_job_status)
      {
        eCAL::pb::rec_server::ClientJobStatus result;
        ToProtobuf(client_job_status, result);
        return result;
      }

      eCAL::pb::rec_server::Measurement ToProtobuf(const eCAL::rec_server::JobHistoryEntry& job_history_entry)
      {
        eCAL::pb::rec_server::Measurement result;
        ToProtobuf(job_history_entry, result);
        return result;
      }

      eCAL::pb::rec_server::Status ToProtobuf(const eCAL::rec_server::RecServerStatus& rec_server_status)
      {
        eCAL::pb::rec_server::Status result;
        ToProtobuf(rec_server_status, result);
        return result;
      }
    }
  }
}
