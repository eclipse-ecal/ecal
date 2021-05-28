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
      ///////////////////////////////
      /// To Protobuf
      ///////////////////////////////

      void ToProtobuf(const eCAL::rec_server::UploadConfig&    upload_config,     eCAL::pb::rec_server::UploadConfig& upload_config_pb)
      {
        // type
        switch (upload_config.type_)
        {
        case eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP:
          upload_config_pb.set_type(eCAL::pb::rec_server::UploadConfig_Type::UploadConfig_Type_InternalFtp);
          break;
        case eCAL::rec_server::UploadConfig::Type::FTP:
          upload_config_pb.set_type(eCAL::pb::rec_server::UploadConfig_Type::UploadConfig_Type_Ftp);
          break;
        default:
          upload_config_pb.set_type(eCAL::pb::rec_server::UploadConfig_Type::UploadConfig_Type_Unknown);
          break;
        }

        // host
        upload_config_pb.set_host(upload_config.host_);

        // port
        upload_config_pb.set_port(upload_config.port_);

        // username
        upload_config_pb.set_username(upload_config.username_);

        // password
        upload_config_pb.set_password(upload_config.password_);

        // root_path
        upload_config_pb.set_root_path(upload_config.root_path_);

        // delete_after_upload
        upload_config_pb.set_delete_after_upload(upload_config.delete_after_upload_);
      }

      void ToProtobuf(const eCAL::rec_server::ClientConfig&    rec_client_config, eCAL::pb::rec_server::RecClientConfig& rec_client_config_pb)
      {
        rec_client_config_pb.clear_host_filter();
        for (const std::string& host : rec_client_config.host_filter_)
        {
          rec_client_config_pb.add_host_filter(host);
        }

        rec_client_config_pb.clear_enabled_addons();
        for (const std::string& addon_id : rec_client_config.enabled_addons_)
        {
          rec_client_config_pb.add_enabled_addons(addon_id);
        }
      }

      void ToProtobuf(const eCAL::rec::RecordMode&             record_mode,       eCAL::pb::rec_server::RecordMode&      record_mode_pb)
      {
        record_mode_pb = ToProtobuf(record_mode);
      }

      void ToProtobuf(const eCAL::rec_server::RecServerConfig& rec_server_config, eCAL::pb::rec_server::RecServerConfig& rec_server_config_pb)
      {
        rec_server_config_pb.set_root_dir(rec_server_config.root_dir_);
        rec_server_config_pb.set_meas_name(rec_server_config.meas_name_);
        rec_server_config_pb.set_max_file_size_mib(rec_server_config.max_file_size_);
        rec_server_config_pb.set_description(rec_server_config.description_);

        rec_server_config_pb.clear_enabled_clients_config();
        for (const auto& client_config_pair : rec_server_config.enabled_clients_config_)
        {
          (*rec_server_config_pb.mutable_enabled_clients_config())[client_config_pair.first] = ToProtobuf(client_config_pair.second);
        }
        rec_server_config_pb.set_pre_buffer_enabled(rec_server_config.pre_buffer_enabled_);
        rec_server_config_pb.set_pre_buffer_length_nsecs(std::chrono::duration_cast<std::chrono::nanoseconds>(rec_server_config.pre_buffer_length_).count());
        rec_server_config_pb.set_built_in_recorder_enabled(rec_server_config.built_in_recorder_enabled_);
        rec_server_config_pb.set_record_mode(ToProtobuf(rec_server_config.record_mode_));

        rec_server_config_pb.clear_listed_topics();
        for (const std::string& topic_name : rec_server_config.listed_topics_)
        {
          rec_server_config_pb.add_listed_topics(topic_name);
        }
      }

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

        // is_uploaded
        measurement_pb.set_is_uploaded(job_history_entry.is_uploaded_);

        // upload_config
        ToProtobuf(job_history_entry.upload_config_, *measurement_pb.mutable_upload_config());
      }

      void ToProtobuf(const std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point>& client_status_pair, const std::string& hostname, eCAL::pb::rec_server::ClientStatus& client_status_pb)
      {
        eCAL::rec::proto_helpers::ToProtobuf(client_status_pair.first, hostname, *client_status_pb.mutable_client_status());
        client_status_pb.set_server_timestamp_of_status(std::chrono::duration_cast<std::chrono::nanoseconds>(client_status_pair.second.time_since_epoch()).count());
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

        // client_statuses
        for (const auto& client_status : rec_server_status.client_statuses_)
        {
          const std::string& hostname                     = client_status.first;
          const auto&        client_status_timestamp_pair = client_status.second;

          (*rec_server_status_pb.mutable_client_statuses())[hostname] = ToProtobuf(client_status_timestamp_pair, hostname);
        }
      }

      eCAL::pb::rec_server::UploadConfig    ToProtobuf(const eCAL::rec_server::UploadConfig&   upload_config)
      {
        eCAL::pb::rec_server::UploadConfig result;
        ToProtobuf(upload_config, result);
        return result;
      }

      eCAL::pb::rec_server::RecClientConfig ToProtobuf(const eCAL::rec_server::ClientConfig&   rec_client_config)
      {
        eCAL::pb::rec_server::RecClientConfig output_pb;
        ToProtobuf(rec_client_config, output_pb);
        return output_pb;
      }

      eCAL::pb::rec_server::RecordMode      ToProtobuf(const eCAL::rec::RecordMode&            record_mode)
      {
        switch (record_mode)
        {
        case eCAL::rec::RecordMode::All:
          return eCAL::pb::rec_server::RecordMode::All;
        case eCAL::rec::RecordMode::Whitelist:
          return eCAL::pb::rec_server::RecordMode::Whitelist;
        case eCAL::rec::RecordMode::Blacklist:
          return eCAL::pb::rec_server::RecordMode::Blacklist;
        default:
          return eCAL::pb::rec_server::RecordMode::All;
        }
      }

      eCAL::pb::rec_server::RecServerConfig ToProtobuf(const eCAL::rec_server::RecServerConfig& rec_server_config)
      {
        eCAL::pb::rec_server::RecServerConfig output_pb;
        ToProtobuf(rec_server_config, output_pb);
        return output_pb;
      }

      eCAL::pb::rec_server::ClientJobStatus ToProtobuf(const eCAL::rec_server::ClientJobStatus& client_job_status)
      {
        eCAL::pb::rec_server::ClientJobStatus result;
        ToProtobuf(client_job_status, result);
        return result;
      }

      eCAL::pb::rec_server::Measurement     ToProtobuf(const eCAL::rec_server::JobHistoryEntry& job_history_entry)
      {
        eCAL::pb::rec_server::Measurement result;
        ToProtobuf(job_history_entry, result);
        return result;
      }

      eCAL::pb::rec_server::ClientStatus    ToProtobuf(const std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point>& client_status_pair, const std::string& hostname)
      {
        eCAL::pb::rec_server::ClientStatus result;
        ToProtobuf(client_status_pair, hostname, result);
        return result;
      }

      eCAL::pb::rec_server::Status          ToProtobuf(const eCAL::rec_server::RecServerStatus& rec_server_status)
      {
        eCAL::pb::rec_server::Status result;
        ToProtobuf(rec_server_status, result);
        return result;
      }


      ///////////////////////////////
      /// From Protobuf
      ///////////////////////////////

      void FromProtobuf(const eCAL::pb::rec_server::UploadConfig&    upload_config_pb,     eCAL::rec_server::UploadConfig&    upload_config)
      {
        // type
        switch (upload_config_pb.type())
        {
        case eCAL::pb::rec_server::UploadConfig::Type::UploadConfig_Type_InternalFtp:
          upload_config.type_ = eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP;
          break;
        case eCAL::pb::rec_server::UploadConfig::Type::UploadConfig_Type_Ftp:
          upload_config.type_ = eCAL::rec_server::UploadConfig::Type::FTP;
          break;
        default:
          upload_config.type_ = eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP;
        }

        // host
        if (upload_config_pb.host() != "")
          upload_config.host_ = upload_config_pb.host();

        // port
        if (upload_config_pb.port() != 0)
          upload_config.port_ = static_cast<uint16_t>(upload_config_pb.port());

        // username
        if (upload_config_pb.username() != "")
          upload_config.username_ = upload_config_pb.username();

        // password
        if (upload_config_pb.password() != "")
          upload_config.password_ = upload_config_pb.password();

        // root_path
        if (upload_config_pb.root_path() != "")
          upload_config.root_path_ = upload_config_pb.root_path();

        // delete_after_upload
        upload_config.delete_after_upload_ = upload_config_pb.delete_after_upload();
      }

      void FromProtobuf(const eCAL::pb::rec_server::RecClientConfig& rec_client_config_pb, eCAL::rec_server::ClientConfig&    rec_client_config)
      {
        rec_client_config.host_filter_.clear();
        for (const auto& host_name : rec_client_config_pb.host_filter())
        {
          rec_client_config.host_filter_.emplace(host_name);
        }

        rec_client_config.enabled_addons_.clear();
        for (const auto& enabled_addon_id : rec_client_config_pb.enabled_addons())
        {
          rec_client_config.enabled_addons_.emplace(enabled_addon_id);
        }
      }

      void FromProtobuf(const eCAL::pb::rec_server::RecordMode&      record_mode_pb,       eCAL::rec::RecordMode&             record_mode)
      {
        record_mode = FromProtobuf(record_mode_pb);
      }

      void FromProtobuf(const eCAL::pb::rec_server::RecServerConfig& rec_server_config_pb, eCAL::rec_server::RecServerConfig& rec_server_config)
      {
        rec_server_config.root_dir_      = rec_server_config_pb.root_dir();
        rec_server_config.meas_name_     = rec_server_config_pb.meas_name();
        rec_server_config.max_file_size_ = rec_server_config_pb.max_file_size_mib();
        rec_server_config.description_   = rec_server_config_pb.description();

        rec_server_config.enabled_clients_config_.clear();
        for (const auto& rec_client_config_pair : rec_server_config_pb.enabled_clients_config())
        {
          rec_server_config.enabled_clients_config_[rec_client_config_pair.first] = FromProtobuf(rec_client_config_pair.second);
        }

        rec_server_config.pre_buffer_enabled_ = rec_server_config_pb.pre_buffer_enabled();
        rec_server_config.pre_buffer_length_ = std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::nanoseconds(rec_server_config_pb.pre_buffer_length_nsecs()));
        rec_server_config.built_in_recorder_enabled_ = rec_server_config_pb.built_in_recorder_enabled();
        rec_server_config.record_mode_ = FromProtobuf(rec_server_config_pb.record_mode());

        rec_server_config.listed_topics_.clear();
        for (const auto& topic_name : rec_server_config_pb.listed_topics())
        {
          rec_server_config.listed_topics_.emplace(topic_name);
        }
      }

      void FromProtobuf(const eCAL::pb::rec_server::ClientJobStatus& client_job_status_pb, eCAL::rec_server::ClientJobStatus& client_job_status)
      {
        client_job_status.client_pid_                 = client_job_status_pb.client_pid();
        eCAL::rec::proto_helpers::FromProtobuf(client_job_status_pb.job_status(), client_job_status.job_status_);
        client_job_status.info_last_command_response_ = { client_job_status_pb.last_command_response_ok(), client_job_status_pb.last_command_response_message() };
      }

      void FromProtobuf(const eCAL::pb::rec_server::Measurement& measurement_pb, eCAL::rec_server::JobHistoryEntry& job_history_entry)
      {
        job_history_entry.local_start_time_ = std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds(measurement_pb.local_timestamp_nsecs())));
        job_history_entry.local_evaluated_job_config_.SetJobId      (measurement_pb.id());
        job_history_entry.local_evaluated_job_config_.SetMeasName   (measurement_pb.local_name());
        job_history_entry.local_evaluated_job_config_.SetMeasRootDir(measurement_pb.local_meas_path());
        job_history_entry.is_deleted_       = measurement_pb.is_deleted();

        job_history_entry.client_statuses_.clear();
        for (const auto& client_job_status_pb : measurement_pb.client_job_statuses())
        {
          job_history_entry.client_statuses_[client_job_status_pb.first] = FromProtobuf(client_job_status_pb.second);
        }

        job_history_entry.is_uploaded_     = measurement_pb.is_uploaded();
        job_history_entry.is_uploaded_     = measurement_pb.is_uploaded();
        FromProtobuf(measurement_pb.upload_config(), job_history_entry.upload_config_);
      }

      void FromProtobuf(const eCAL::pb::rec_server::ClientStatus& client_status_pb, std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point>& client_status_pair, std::string& hostname)
      {
        eCAL::rec::proto_helpers::FromProtobuf(client_status_pb.client_status(), hostname, client_status_pair.first);
        client_status_pair.second = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(client_status_pb.server_timestamp_of_status()));
      }

      void FromProtobuf(const eCAL::pb::rec_server::Status& rec_server_status_pb, eCAL::rec_server::RecServerStatus& rec_server_status)
      {
        rec_server_status.config_path_ = rec_server_status_pb.config_path();
        rec_server_status.activated_ = rec_server_status_pb.activated();
        rec_server_status.recording_meas_id_ = rec_server_status_pb.recording_meas_id();

        rec_server_status.job_history_.clear();
        for (const auto& meas_history_entry : rec_server_status_pb.measurement_history())
        {
          rec_server_status.job_history_.push_back(FromProtobuf(meas_history_entry));
        }

        rec_server_status.client_statuses_.clear();
        for (const auto& client_status_pair_pb : rec_server_status_pb.client_statuses())
        {
          rec_server_status.client_statuses_[client_status_pair_pb.first] = FromProtobuf(client_status_pair_pb.second).first;
        }
      }

      eCAL::rec_server::UploadConfig    FromProtobuf(const eCAL::pb::rec_server::UploadConfig& upload_config_pb)
      {
        eCAL::rec_server::UploadConfig output;
        FromProtobuf(upload_config_pb, output);
        return output;
      }
      
      eCAL::rec_server::ClientConfig    FromProtobuf(const eCAL::pb::rec_server::RecClientConfig& rec_client_config_pb)
      {
        eCAL::rec_server::ClientConfig output;
        FromProtobuf(rec_client_config_pb, output);
        return output;
      }
      
      eCAL::rec::RecordMode             FromProtobuf(const eCAL::pb::rec_server::RecordMode& record_mode_pb)
      {
        switch (record_mode_pb)
        {
        case eCAL::pb::rec_server::RecordMode::All:
          return eCAL::rec::RecordMode::All;
        case eCAL::pb::rec_server::RecordMode::Whitelist:
          return eCAL::rec::RecordMode::Whitelist;
        case eCAL::pb::rec_server::RecordMode::Blacklist:
          return eCAL::rec::RecordMode::Blacklist;
        default:
          return eCAL::rec::RecordMode::All;
        }
      }

      eCAL::rec_server::RecServerConfig FromProtobuf(const eCAL::pb::rec_server::RecServerConfig& rec_server_config_pb)
      {
        eCAL::rec_server::RecServerConfig output;
        FromProtobuf(rec_server_config_pb, output);
        return output;
      }

      eCAL::rec_server::ClientJobStatus FromProtobuf(const eCAL::pb::rec_server::ClientJobStatus& client_job_status_pb)
      {
        eCAL::rec_server::ClientJobStatus output;
        FromProtobuf(client_job_status_pb, output);
        return output;
      }

      eCAL::rec_server::JobHistoryEntry FromProtobuf(const eCAL::pb::rec_server::Measurement&     measurement_pb)
      {
        eCAL::rec_server::JobHistoryEntry output;
        FromProtobuf(measurement_pb, output);
        return output;
      }

      std::pair<std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point>, std::string> FromProtobuf(const eCAL::pb::rec_server::ClientStatus& client_status_pb)
      {
        std::pair<eCAL::rec::RecorderStatus, eCAL::Time::ecal_clock::time_point> client_status_pair;
        std::string hostname;
        FromProtobuf(client_status_pb, client_status_pair, hostname);
        return std::make_pair(client_status_pair, hostname);
      }

      eCAL::rec_server::RecServerStatus FromProtobuf(const eCAL::pb::rec_server::Status&          rec_server_status_pb)
      {
        eCAL::rec_server::RecServerStatus output;
        FromProtobuf(rec_server_status_pb, output);
        return output;
      }

      ///////////////////////////////
      /// Service From Protobuf
      ///////////////////////////////

      void FromProtobuf(const eCAL::pb::rec_server::ServiceResult& service_result_pb, eCAL::rec::Error& error)
      {
        error = FromProtobuf(service_result_pb);
      }

      eCAL::rec::Error FromProtobuf(const eCAL::pb::rec_server::ServiceResult& service_result_pb)
      {
        const std::string&          message = service_result_pb.info_message();
        eCAL::rec::Error::ErrorCode error_code;

        switch (service_result_pb.error_code())
        {
        case ::eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error:
          error_code = eCAL::rec::Error::ErrorCode::OK;
          break;
        case ::eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_parameters:
          error_code = eCAL::rec::Error::ErrorCode::PARAMETER_ERROR;
          break;
        case ::eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal:
          error_code = eCAL::rec::Error::ErrorCode::GENERIC_ERROR;
          break;
        default:
          error_code = eCAL::rec::Error::ErrorCode::GENERIC_ERROR;
          break;
        }

        return eCAL::rec::Error(error_code, message);
      }
    }
  }
}
