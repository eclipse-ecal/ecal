#pragma once

#include "abstract_connection.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/msg/protobuf/client.h>
#include <ecal/pb/sys/client_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <mutex>

namespace eCAL
{
  namespace sys
  {
    class RemoteConnection : public AbstractConnection
    {
    public:
      RemoteConnection(const std::string& hostname);
      ~RemoteConnection() override;

      std::vector<int32_t> StartTasks(const std::vector<eCAL::sys_client::StartTaskParameters>& task_list) override;

      std::vector<bool>    StopTasks (const std::vector<eCAL::sys_client::StopTaskParameters>& task_list) override;

      std::vector<std::vector<int32_t>> MatchTasks(const std::vector<eCAL::sys_client::Task>& task_list) override;

    private:
      eCAL::sys_client::Task EvaluateEcalParserHostFunctions(const eCAL::sys_client::Task& task, std::chrono::system_clock::time_point now) const;

    //////////////////////////////////////////
    // Member Variables
    //////////////////////////////////////////
    private:
      std::mutex connection_mutex_;
      eCAL::protobuf::CServiceClient<eCAL::pb::sys_client::EcalSysClientService> sys_client_service_;
    };
  }
}