#include "remote_connection.h"

#include <sys_client_core/proto_helpers.h>

#include <EcalParser/EcalParser.h>

namespace eCAL
{
  namespace sys
  {

    RemoteConnection::RemoteConnection(const std::string& hostname)
      : AbstractConnection(hostname)
    {
      sys_client_service_.SetHostName(hostname);
    }

    RemoteConnection::~RemoteConnection()
    {}

    std::vector<int32_t> RemoteConnection::StartTasks(const std::vector<eCAL::sys_client::StartTaskParameters>& task_list)
    {
      std::vector<eCAL::sys_client::StartTaskParameters> evaluated_task_list = task_list;
      auto now = std::chrono::system_clock::now();

      for (auto& start_task_param : evaluated_task_list)
      {
        start_task_param.task = EvaluateEcalParserHostFunctions(start_task_param.task, now);
      }

      eCAL::pb::sys_client::StartTaskRequest start_task_request_pb = eCAL::sys_client::proto_helpers::ToProtobuf(evaluated_task_list);
      eCAL::pb::sys_client::TaskResponse     response_pb;

      if (CallRemoteEcalsysService("StartTasks", start_task_request_pb, response_pb))
      {
        std::vector<int32_t> return_values;
        return_values.reserve(evaluated_task_list.size());

        for(const auto& task_response_pb : response_pb.responses())
        {
          return_values.push_back(task_response_pb.pid());
        }

        return return_values;
      }
      else
      {
        return {};
      }
    }

    std::vector<bool> RemoteConnection::StopTasks (const std::vector<eCAL::sys_client::StopTaskParameters>& task_list)
    {
      std::vector<eCAL::sys_client::StopTaskParameters> evaluated_task_list = task_list;
      auto now = std::chrono::system_clock::now();

      for (auto& start_task_param : evaluated_task_list)
      {
        start_task_param.task = EvaluateEcalParserHostFunctions(start_task_param.task, now);
      }

      eCAL::pb::sys_client::StopTaskRequest stop_task_request_pb = eCAL::sys_client::proto_helpers::ToProtobuf(evaluated_task_list);
      eCAL::pb::sys_client::TaskResponse    response_pb;

      if (CallRemoteEcalsysService("StopTasks", stop_task_request_pb, response_pb))
      {
        std::vector<bool> return_values;
        return_values.reserve(evaluated_task_list.size());

        for(const auto& task_response_pb : response_pb.responses())
        {
          return_values.push_back(task_response_pb.result() == eCAL::pb::sys_client::eServiceResult::success);
        }

        return return_values;
      }
      else
      {
        return {};
      }
    }

    std::vector<std::vector<int32_t>> RemoteConnection::MatchTasks(const std::vector<eCAL::sys_client::Task>& task_list)
    {
      std::vector<eCAL::sys_client::Task> evaluated_task_list = task_list;
      auto now = std::chrono::system_clock::now();

      for (auto& task : evaluated_task_list)
      {
        task = EvaluateEcalParserHostFunctions(task, now);
      }

      eCAL::pb::sys_client::TaskList          task_list_pb = eCAL::sys_client::proto_helpers::ToProtobuf(evaluated_task_list);
      eCAL::pb::sys_client::MatchTaskResponse response_pb;

      if (CallRemoteEcalsysService("MatchTasks", task_list_pb, response_pb))
      {
        return eCAL::sys_client::proto_helpers::FromProtobuf(response_pb);
      }
      else
      {
        return {};
      }
    }

    eCAL::sys_client::Task RemoteConnection::EvaluateEcalParserHostFunctions(const eCAL::sys_client::Task& task, std::chrono::system_clock::time_point now) const
    {
      eCAL::sys_client::Task evaluated_task = task;

      evaluated_task.path                    = EcalParser::Evaluate(evaluated_task.path, false, now);
      evaluated_task.arguments               = EcalParser::Evaluate(evaluated_task.arguments, false, now);
      evaluated_task.working_dir             = EcalParser::Evaluate(evaluated_task.working_dir, false, now);

      evaluated_task.runner.path             = EcalParser::Evaluate(evaluated_task.runner.path, false, now);
      evaluated_task.runner.arguments        = EcalParser::Evaluate(evaluated_task.runner.arguments, false, now);
      evaluated_task.runner.default_task_dir = EcalParser::Evaluate(evaluated_task.runner.default_task_dir, false, now);

      return evaluated_task;
    }

    bool RemoteConnection::CallRemoteEcalsysService(const std::string&               method_name
                                                  , const google::protobuf::Message& request
                                                  , google::protobuf::Message&       response)
    {
      std::lock_guard<decltype(connection_mutex_)> connection_lock(connection_mutex_);

      eCAL::ServiceResponseVecT service_response_vec;
      constexpr int timeout_ms = 1000;

      if (sys_client_service_.Call(method_name, request.SerializeAsString(), timeout_ms, &service_response_vec))
      {
        if (service_response_vec.size() > 0)
        {
          response.ParseFromString(service_response_vec[0].response);
          return true;
        }
      }
      return false;
    }

  }
}