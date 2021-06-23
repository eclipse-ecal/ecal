/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "command.h"


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505) // disable tclap warning (unreferenced local function has been removed)
#endif
#include "tclap/CmdLine.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <custom_tclap/advanced_tclap_output.h>
#include <custom_tclap/fuzzy_value_switch_arg_bool.h>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {
      class Status : public Command
      {
      //////////////////////////////////////////////
      /// Job History Entry evaluation methods
      //////////////////////////////////////////////
        struct CmdLine
        {
          CmdLine()
            : cmd                  ("", ' ', "", false)
            , advanced_tclap_output(std::vector<std::ostream*>{ &tclap_output_stream }, 75)

            , unlabled_arg            ("unlabled_arg", "", false, "")
            , meas_id_arg             ("m", "meas",   "Print detailed information about the given measurement.", false, "", "ID")
            , rec_client_arg          ("h", "host", "Print detailed information about the recorder client from the given host. If that client has a loaded addon, information about all addons are also printed.", false, "", "HOSTNAME")
          {
            std::vector<TCLAP::Arg*> arg_vector =
            {
              &meas_id_arg,
              &rec_client_arg,
              &unlabled_arg,
            };

            for (auto arg_iterator = arg_vector.rbegin(); arg_iterator != arg_vector.rend(); ++arg_iterator)
              cmd.add(*arg_iterator);

            advanced_tclap_output.setArgumentHidden(&unlabled_arg, true);
            cmd.setOutput(&advanced_tclap_output);
          }

          void parse(const std::vector<std::string>& argv)
          {
            std::vector<std::string> argv_with_program_name;
            argv_with_program_name.reserve(argv.size() + 1);
            argv_with_program_name.push_back("");

            for (size_t i = 0; i < argv.size(); i++)
              argv_with_program_name.push_back(argv[i]);

            cmd.parse(argv_with_program_name);
          }

          std::string help()
          {
            tclap_output_stream.clear();
            advanced_tclap_output.usage(cmd);
            return tclap_output_stream.str();
          }

          TCLAP::CmdLine                   cmd;
          std::stringstream                tclap_output_stream;
          CustomTclap::AdvancedTclapOutput advanced_tclap_output;

          TCLAP::UnlabeledMultiArg<std::string> unlabled_arg;
          TCLAP::ValueArg<std::string>          meas_id_arg;
          TCLAP::ValueArg<std::string>          rec_client_arg;
        };

      //////////////////////////////////////////////
      /// Command functions (-> overrides)
      //////////////////////////////////////////////
      public:
        std::string Usage()    const override;
        std::string Help()     const override;
        std::string Example()  const override;

        eCAL::rec::Error Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const override;
        eCAL::rec::Error Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const override;

      private:
        eCAL::rec::Error Execute(const eCAL::rec_server::RecServerStatus& status, const eCAL::rec_server::RecServerConfig& config, const std::vector<std::string>& argv) const;
        eCAL::rec::Error Execute(const eCAL::rec_server::RecServerStatus& status, const eCAL::rec_server::RecServerConfig& config, const std::vector<std::string>& argv, std::ostream& ostream) const;

      //////////////////////////////////////////////
      /// Print various information
      //////////////////////////////////////////////
      
      public:
        eCAL::rec::Error printStatus(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service) const;

        eCAL::rec::Error printJobState(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, int64_t meas_id) const;

        eCAL::rec::Error printClientInformation(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& client_hostname) const;

      private:
        void printRecServerStatus(const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const;

        void printJobHistory(const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const;

        void printClientList(const eCAL::rec_server::RecServerStatus& status, const eCAL::rec_server::RecServerConfig& config, std::ostream& ostream) const;

        eCAL::rec::Error printJobState(const std::string& job_id_string,                           const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const;
        eCAL::rec::Error printJobState(int64_t job_id,                                             const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const;
        eCAL::rec::Error printJobState(const eCAL::rec_server::JobHistoryEntry& job_history_entry, const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const;

        eCAL::rec::Error printClientInformation(const std::string& hostname, const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const;

      //////////////////////////////////////////////
      /// Job History Entry evaluation methods
      //////////////////////////////////////////////
      private:

        eCAL::rec::JobState combinedJobState(const eCAL::rec_server::JobHistoryEntry& job_history_entry) const;

        std::pair<std::chrono::steady_clock::duration, int64_t> combinedLength(const eCAL::rec_server::JobHistoryEntry& job_history_entry) const;

        int64_t combinedUnflushedFrames(const eCAL::rec_server::JobHistoryEntry& job_history_entry) const;

        eCAL::rec::UploadStatus combinedUploadStatus(const eCAL::rec_server::JobHistoryEntry& job_history_entry) const;

        std::pair<bool, std::string> combinedInfo(const eCAL::rec_server::JobHistoryEntry& job_history_entry, const eCAL::rec_server::RecServerStatus& status) const;

        std::pair<bool, std::string> displayedInfo(const eCAL::rec_server::ClientJobStatus& client_job_status) const;

        std::string getHumanReadableAddonName(const std::string& hostname, const std::string& addon_id, const eCAL::rec_server::RecServerStatus& status) const;

        std::string timeToString(eCAL::Time::ecal_clock::time_point time) const;
        std::string timeToString(std::chrono::system_clock::time_point time) const;

        std::string combinedJobStateToString(eCAL::rec::JobState combined_job_state, const eCAL::rec_server::JobHistoryEntry& job_history_entry) const;

        std::pair<std::string, eCAL::rec::JobState> recClientStateToString(eCAL::rec::RecorderStatus rec_client_status) const;
      };
    }
  }
}
