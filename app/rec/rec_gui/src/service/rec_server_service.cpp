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

#include "rec_server_service.h"

#include <rec_server_core/proto_helpers.h>

#include <qecalrec.h>

RecServerService::RecServerService()
  : eCAL::pb::rec_server::EcalRecServerService()
{}

RecServerService::~RecServerService()
{}

void RecServerService::LoadConfigFile(::google::protobuf::RpcController*               /*controller*/
                                    , const ::eCAL::pb::rec_server::LoadConfigRequest* request
                                    , ::eCAL::pb::rec_server::ServiceResult*           response
                                    , ::google::protobuf::Closure*                     /*done*/)
{
  const std::string& path = request->config_path();
  
  if (path.empty())
  {
    response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_parameters);
    response->set_info_message("No config path given");
    return;
  }

  bool success = false;
  {
    bool is_recording = false;
    QMetaObject::invokeMethod(QEcalRec::instance(), "recording", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, is_recording));
    if (is_recording)
    {
      QMetaObject::invokeMethod(QEcalRec::instance(), "stopRecording", Qt::BlockingQueuedConnection);
    }
    QMetaObject::invokeMethod(QEcalRec::instance(), "loadConfigFromFile", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(const std::string&, path), Q_ARG(bool, true));
  }

  if (success)
  {
    response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
    response->set_info_message("Successfully loaded config file.");
  }
  else
  {
    response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
    response->set_info_message("Failed loading config file.");
  }
}

void RecServerService::Activate(::google::protobuf::RpcController*                 /*controller*/
                                   , const ::eCAL::pb::rec_server::GenericRequest* /*request*/
                                   , ::eCAL::pb::rec_server::ServiceResult*        response
                                   , ::google::protobuf::Closure*                  /*done*/)
{
  bool success = false;
  {
    QMetaObject::invokeMethod(QEcalRec::instance(), "connectToEcal", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, true));
  }

  if (success)
  {
    response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
    response->set_info_message("Successfully activated clients.");
  }
  else
  {
    response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
    response->set_info_message("Failed to activate clients.");
  }
}

void RecServerService::DeActivate(::google::protobuf::RpcController*               /*controller*/
                                   , const ::eCAL::pb::rec_server::GenericRequest* /*request*/
                                   , ::eCAL::pb::rec_server::ServiceResult*        response
                                   , ::google::protobuf::Closure*                  /*done*/)
{
  bool success = false;
  {
    QMetaObject::invokeMethod(QEcalRec::instance(), "setConnectionToClientsActive", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, false), Q_ARG(bool, true));
  }

  if (success)
  {
    response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
    response->set_info_message("Successfully de-activated clients");
  }
  else
  {
    response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
    response->set_info_message("Failed to de-activate clients");
  }
}

void RecServerService::StartRecording(::google::protobuf::RpcController*           /*controller*/
                                   , const ::eCAL::pb::rec_server::GenericRequest* /*request*/
                                   , ::eCAL::pb::rec_server::JobStartedResponse*   response
                                   , ::google::protobuf::Closure*                  /*done*/)
{
  bool success = false;
  {
    QMetaObject::invokeMethod(QEcalRec::instance(), "startRecording", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, true));
  }

  if (success)
  {
    std::list<eCAL::rec_server::JobHistoryEntry> job_history;
    QMetaObject::invokeMethod(QEcalRec::instance(), "jobHistory", Qt::BlockingQueuedConnection, Q_RETURN_ARG(std::list<eCAL::rec_server::JobHistoryEntry>, job_history));

    int64_t meas_id = 0;
    if (job_history.size() > 0)
    {
      meas_id = job_history.back().local_evaluated_job_config_.GetJobId();
    }
    response->set_meas_id(meas_id);
    response->mutable_service_result()->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
    response->mutable_service_result()->set_info_message("Successfully started recording with ID " + std::to_string(meas_id));
  }
  else
  {
    response->mutable_service_result()->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
    response->mutable_service_result()->set_info_message("Unable to start recording");
  }
}

void RecServerService::StopRecording(::google::protobuf::RpcController*            /*controller*/
                                   , const ::eCAL::pb::rec_server::GenericRequest* /*request*/
                                   , ::eCAL::pb::rec_server::ServiceResult*        response
                                   , ::google::protobuf::Closure*                  /*done*/)
{
  bool success = false;
  {
    QMetaObject::invokeMethod(QEcalRec::instance(), "stopRecording", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, true));
  }

  if (success)
  {
    response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
    response->set_info_message("Successfully stopped recording");
  }
  else
  {
    response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
    response->set_info_message("Unable to stop recording");
  }
}

void RecServerService::SaveBuffer(::google::protobuf::RpcController*               /*controller*/
                                   , const ::eCAL::pb::rec_server::GenericRequest* /*request*/
                                   , ::eCAL::pb::rec_server::JobStartedResponse*   response
                                   , ::google::protobuf::Closure*                  /*done*/)
{
  bool success = false;
  {
    QMetaObject::invokeMethod(QEcalRec::instance(), "savePreBufferedData", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, success), Q_ARG(bool, true));
  }

  if (success)
  {
    std::list<eCAL::rec_server::JobHistoryEntry> job_history;
    QMetaObject::invokeMethod(QEcalRec::instance(), "jobHistory", Qt::BlockingQueuedConnection, Q_RETURN_ARG(std::list<eCAL::rec_server::JobHistoryEntry>, job_history));

    int64_t meas_id = 0;
    if (job_history.size() > 0)
    {
      meas_id = job_history.back().local_evaluated_job_config_.GetJobId();
    }
    response->set_meas_id(meas_id);
    response->mutable_service_result()->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
    response->mutable_service_result()->set_info_message("Successfully saved buffer with ID " + std::to_string(meas_id));
  }
  else
  {
    response->mutable_service_result()->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
    response->mutable_service_result()->set_info_message("Unable to save buffer");
  }
}

void RecServerService::UploadMeasurement(::google::protobuf::RpcController*                  /*controller*/
                                   , const ::eCAL::pb::rec_server::GenericMeasurementRequest* request
                                   , ::eCAL::pb::rec_server::ServiceResult*                  response
                                   , ::google::protobuf::Closure*                            /*done*/)
{
  if (request->meas_id() != 0)
  {
    // Upload specific measurement
    eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
    QMetaObject::invokeMethod(QEcalRec::instance(), "uploadMeasurement", Qt::BlockingQueuedConnection, Q_RETURN_ARG(eCAL::rec::Error, error), Q_ARG(int64_t, request->meas_id()));
    
    if (error)
    {
      response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
      response->set_info_message("Successfully started uploading measurement");
    }
    else
    {
      response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
      response->set_info_message("Failed to upload measurement");
    }
  }
  else
  {
    // Upload all measurements
    int num_measurements_uploaded = 0;
    QMetaObject::invokeMethod(QEcalRec::instance(), "uploadNonUploadedMeasurements", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, num_measurements_uploaded), Q_ARG(bool, true));

    response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
    response->set_info_message("Started uploading " + std::to_string(num_measurements_uploaded) + " measurements");
  }
}

void RecServerService::AddComment(::google::protobuf::RpcController*                  /*controller*/
                                   , const ::eCAL::pb::rec_server::AddCommentRequest* request
                                   , ::eCAL::pb::rec_server::ServiceResult*           response
                                   , ::google::protobuf::Closure*                     /*done*/)
{
  if (request->meas_id() != 0)
  {
    // Add comment to specific measurement
    eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
    QMetaObject::invokeMethod(QEcalRec::instance(), "addComment", Qt::BlockingQueuedConnection, Q_RETURN_ARG(eCAL::rec::Error, error), Q_ARG(int64_t, request->meas_id()), Q_ARG(std::string, request->comment()), Q_ARG(bool, true));

    if (!error)
    {
      response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
      response->set_info_message("Successfully added comment");
      return;
    }
    else
    {
      response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
      response->set_info_message("Failed to add comment: " + error.ToString());
      return;
    }
  }
  else
  {
    // Add comment to last measurements

    std::list<eCAL::rec_server::JobHistoryEntry> job_history;
    QMetaObject::invokeMethod(QEcalRec::instance(), "jobHistory", Qt::BlockingQueuedConnection, Q_RETURN_ARG(std::list<eCAL::rec_server::JobHistoryEntry>, job_history));

    if (job_history.size() > 0)
    {
      int64_t last_meas_id = job_history.back().local_evaluated_job_config_.GetJobId();

      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      QMetaObject::invokeMethod(QEcalRec::instance(), "addComment", Qt::BlockingQueuedConnection, Q_RETURN_ARG(eCAL::rec::Error, error), Q_ARG(int64_t, last_meas_id), Q_ARG(std::string, request->comment()), Q_ARG(bool, true));

      if (!error)
      {
        response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
        response->set_info_message("Successfully added comment to measurement " + std::to_string(last_meas_id));
        return;
      }
      else
      {
        response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
        response->set_info_message("Failed to add comment to measurement " + std::to_string(last_meas_id) + ": " + error.ToString());
        return;
      }
    }
    else
    {
      response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
      response->set_info_message("Failed to add comment to last measurement. The measurement history is empty.");
      return;
    }
  }
}

void RecServerService::DeleteMeasurement(::google::protobuf::RpcController*                  /*controller*/
                                   , const ::eCAL::pb::rec_server::GenericMeasurementRequest* request
                                   , ::eCAL::pb::rec_server::ServiceResult*                  response
                                   , ::google::protobuf::Closure*                            /*done*/)
{
  if (request->meas_id() != 0)
  {
    // Upload delete specific measurement
    eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
    QMetaObject::invokeMethod(QEcalRec::instance(), "deleteMeasurement", Qt::BlockingQueuedConnection, Q_RETURN_ARG(eCAL::rec::Error, error), Q_ARG(int64_t, request->meas_id()), Q_ARG(bool, true));
    
    if (!error)
    {
      response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
      response->set_info_message("Successfully deleted measurement");
    }
    else
    {
      response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
      response->set_info_message("Failed to delete measurement: " + error.ToString());
    }
  }
  else
  {
    std::list<eCAL::rec_server::JobHistoryEntry> job_history;
    QMetaObject::invokeMethod(QEcalRec::instance(), "jobHistory", Qt::BlockingQueuedConnection, Q_RETURN_ARG(std::list<eCAL::rec_server::JobHistoryEntry>, job_history));

    if (job_history.size() > 0)
    {
      int64_t last_meas_id = job_history.back().local_evaluated_job_config_.GetJobId();

      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      QMetaObject::invokeMethod(QEcalRec::instance(), "deleteMeasurement", Qt::BlockingQueuedConnection, Q_RETURN_ARG(eCAL::rec::Error, error), Q_ARG(int64_t, last_meas_id), Q_ARG(bool, true));

      if (!error)
      {
        response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_no_error);
        response->set_info_message("Successfully deleted measurement");
      }
      else
      {
        response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
        response->set_info_message("Failed to delete measurement: " + error.ToString());
      }
    }
    else
    {
      response->set_error_code(eCAL::pb::rec_server::ServiceResult::ErrorCode::ServiceResult_ErrorCode_error_internal);
      response->set_info_message("Failed to delete last measurement. The measurement history is empty.");
      return;
    }
  }
}


void RecServerService::GetStatus(::google::protobuf::RpcController*               /*controller*/
                                  , const ::eCAL::pb::rec_server::GenericRequest* /*request*/
                                  , ::eCAL::pb::rec_server::Status*               response
                                  , ::google::protobuf::Closure*                  /*done*/)
{
  eCAL::rec_server::RecServerStatus status;
  QMetaObject::invokeMethod(QEcalRec::instance(), "status", Qt::BlockingQueuedConnection, Q_RETURN_ARG(eCAL::rec_server::RecServerStatus, status));

  eCAL::rec_server::proto_helpers::ToProtobuf(status, *response);
}
