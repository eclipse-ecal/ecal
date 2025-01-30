/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "status.h"

#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

#include <ecal_utils/string.h>
#include <ecal_utils/ecal_utils.h>

// termcolor pollutes everything by including Windows.h in the header file.
#ifdef _WIN32
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
#endif // _WIN32

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 4800) // disable termcolor warnings
#endif

  #include <cstdint> // Needed for termcolor. Currently (2023-10-26) termcolor does not include this header file, but it is needed for Ubuntu 23.10 / gcc 13.2: https://github.com/ikalnytskyi/termcolor/pull/72
  #include <termcolor/termcolor.hpp>

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#include "table_printer.h"
#include <bytes_to_pretty_string_utils.h>


namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {
      //////////////////////////////////////////////
      /// Command functions (-> overrides)
      //////////////////////////////////////////////
 
      std::string Status::Usage() const
      {
        return "[--meas <MeasID>] | [--host <Hostname>]";
      }

      std::string Status::Help() const
      {
        return "Prints the recorder status. If no argument is provided, the recorder status, job history and client list ist printed. To get more detailed information, provide the measurement ID or hostname. The --meas and --host specifiers can be omitted, if the given measurement id / hostname is unique.";
      }

      std::string Status::Example() const
      {
        return "--meas 32457132";
      }

      eCAL::rec::Error Status::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const
      {
        auto status = rec_server_instance->GetStatus();
        auto config = rec_server_instance->GetConfig();

        return Execute(status, config, argv);
      }

      eCAL::rec::Error Status::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const
      {
        eCAL::rec_server::RecServerStatus status;
        eCAL::rec_server::RecServerConfig config;

        {
          auto error = GetRemoteStatus(hostname, remote_rec_server_service, status);
          if (error)
            return error;
        }

        {
          auto error = GetRemoteConfig(hostname, remote_rec_server_service, config);
          if (error)
            return error;
        }

        return Execute(status, config, argv);
      }

      eCAL::rec::Error Status::Execute(const eCAL::rec_server::RecServerStatus& status, const eCAL::rec_server::RecServerConfig& config, const std::vector<std::string>& argv) const
      {
        std::ostream& ss = std::cout;
        auto error = Execute(status, config, argv, ss);
        return error;
      }

      eCAL::rec::Error Status::Execute(const eCAL::rec_server::RecServerStatus& status, const eCAL::rec_server::RecServerConfig& config, const std::vector<std::string>& argv, std::ostream& ostream) const
      {
        CmdLine cmd_line;
        cmd_line.parse(argv);

        if (cmd_line.unlabled_arg.isSet() && !cmd_line.unlabled_arg.getValue().empty())
        {
          std::cerr << "WARNING: Unrecognized parameters: " + EcalUtils::CommandLine::ToCommandLine(cmd_line.unlabled_arg.getValue()) << "\n";
        }

        if (cmd_line.meas_id_arg.isSet())
        {
          return printJobState(cmd_line.meas_id_arg.getValue(), status, ostream);
        }
        else if (cmd_line.rec_client_arg.isSet())
        {
          return printClientInformation(cmd_line.rec_client_arg.getValue(), status, ostream);
        }
        else if (!cmd_line.unlabled_arg.getValue().empty())
        {
          if (cmd_line.unlabled_arg.getValue().size() > 1)
          {
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS);
          }
          else
          {
            if (!printJobState(cmd_line.unlabled_arg.getValue()[0], status, ostream))
              return eCAL::rec::Error::ErrorCode::OK;
            else if (!printClientInformation(cmd_line.unlabled_arg.getValue()[0], status, ostream))
              return eCAL::rec::Error::ErrorCode::OK;
            else
              return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, cmd_line.unlabled_arg.getValue()[0]);
          }
        }
        else
        {
          // Code duplicated to printStatus function:
          printRecServerStatus(status, ostream);
          ostream << "\n";
          printClientList(status, config, ostream);
          ostream << "\n";
          printJobHistory(status, ostream);

          return eCAL::rec::Error::OK;
        }
      }

      //////////////////////////////////////////////
      /// Print various information
      //////////////////////////////////////////////
      
      eCAL::rec::Error Status::printStatus(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service) const
      {
        eCAL::rec_server::RecServerStatus status;
        eCAL::rec_server::RecServerConfig config;

        {
          auto error = GetRemoteStatus(hostname, remote_rec_server_service, status);
          if (error)
            return error;
        }

        {
          auto error = GetRemoteConfig(hostname, remote_rec_server_service, config);
          if (error)
            return error;
        }

        // Code duplicated from execute function:
        printRecServerStatus(status, std::cout);
        std::cout << "\n";
        printClientList(status, config, std::cout);
        std::cout << "\n";
        printJobHistory(status, std::cout);

        return eCAL::rec::Error::ErrorCode::OK;
      }

      eCAL::rec::Error Status::printJobState(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, int64_t meas_id) const
      {
        eCAL::rec_server::RecServerStatus status;
        auto error = GetRemoteStatus(hostname, remote_rec_server_service, status);

        if (error)
          return error;

        return printJobState(meas_id, status, std::cout);
      }

      eCAL::rec::Error Status::printClientInformation(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::string& client_hostname) const
      {
        eCAL::rec_server::RecServerStatus status;
        auto error = GetRemoteStatus(hostname, remote_rec_server_service, status);

        if (error)
          return error;

        return printClientInformation(client_hostname, status, std::cout);
      }


      void Status::printRecServerStatus(const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const
      {
        ostream << "Config path: " << (status.config_path_.empty() ? "none" : status.config_path_) << "\n";

        ostream << "Activated:   ";
        if (status.activated_)
          ostream << termcolor::green;
        else
          ostream << termcolor::yellow;
        ostream << (status.activated_ ? "Yes" : "No");
        ostream << termcolor::reset << "\n";

        ostream << "Status:      ";
        if (status.recording_meas_id_ != 0)
          ostream << termcolor::red << "REC" << termcolor::reset << " (ID: " << status.recording_meas_id_ << ")";
        else
          ostream << "Stopped";
        ostream << termcolor::reset << "\n";
      }

      void Status::printJobHistory(const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const
      {
        enum class Column : int
        {
          ID,
          NAME,
          STATUS,
          LENGTH,
          INFO,
          COLUMN_COUNT,
        };

        std::vector<std::vector<table_printer::TableEntry>> rows;
        rows.reserve(status.job_history_.size());

        std::vector<table_printer::TableEntry> header_data((int)Column::COLUMN_COUNT);
        header_data[(int)Column::ID]     = table_printer::TableEntry("ID");
        header_data[(int)Column::NAME]   = table_printer::TableEntry("Name");
        header_data[(int)Column::STATUS] = table_printer::TableEntry("Status");
        header_data[(int)Column::LENGTH] = table_printer::TableEntry("Length");
        header_data[(int)Column::INFO]   = table_printer::TableEntry("Info");

        rows.push_back(header_data);

        for (const auto& job_history_entry : status.job_history_)
        {
          std::vector<table_printer::TableEntry> data((int)Column::COLUMN_COUNT);
          data[(int)Column::ID]     = table_printer::TableEntry(std::to_string(job_history_entry.local_evaluated_job_config_.GetJobId()), table_printer::Alignment::ALIGN_RIGHT);
          data[(int)Column::NAME]   = table_printer::TableEntry(job_history_entry.local_evaluated_job_config_.GetMeasName());

          auto combined_state = combinedJobState(job_history_entry);
          table_printer::TableEntry job_state_entry(combinedJobStateToString(combined_state, job_history_entry));

          if (combined_state == eCAL::rec::JobState::Recording)
            job_state_entry.text_color = eCAL::rec_cli::table_printer::Color::RED;

          data[(int)Column::STATUS] = job_state_entry;

          const auto combined_length = combinedLength(job_history_entry);
          std::stringstream length_ss;
          length_ss << std::fixed << std::setprecision(1) << std::chrono::duration_cast<std::chrono::duration<double>>(combined_length.first).count() << " s / " << combined_length.second << " frames";
          data[(int)Column::LENGTH] = table_printer::TableEntry(length_ss.str());

          const auto combined_info = combinedInfo(job_history_entry, status);
          data[(int)Column::INFO]   = table_printer::TableEntry(combined_info.second);
          if (!combined_info.first)
          {
            data[(int)Column::INFO].background_color = table_printer::Color::RED;
          }

          if (job_history_entry.is_deleted_)
          {
            for (auto& table_entry : data)
            {
              table_entry.format_flags |= table_printer::FormatFlag::CROSSED; // Unix only, Windows does not support "crossed" formatting
            }
          }

          rows.push_back(data);
        }

        table_printer::printTable(rows, ostream);
      }

      void Status::printClientList(const eCAL::rec_server::RecServerStatus& status, const eCAL::rec_server::RecServerConfig& config, std::ostream& ostream) const
      {
        enum class Column : int
        {
          CLIENT_HOSTNAME,
          TIME,
          BUFFER,
          STATE,
          INFO,
          COLUMN_COUNT
        };

        std::vector<std::vector<eCAL::rec_cli::table_printer::TableEntry>> client_table;
        client_table.reserve(status.client_statuses_.size() + 1);

        {
          std::vector<table_printer::TableEntry> header_data((int)Column::COLUMN_COUNT);
          header_data[(int)Column::CLIENT_HOSTNAME] = table_printer::TableEntry("Recorder");
          header_data[(int)Column::TIME]            = table_printer::TableEntry("Time");
          header_data[(int)Column::BUFFER]          = table_printer::TableEntry("Buffer");
          header_data[(int)Column::STATE]           = table_printer::TableEntry("State");
          header_data[(int)Column::INFO]            = table_printer::TableEntry("Info");

          client_table.push_back(std::move(header_data));
        }

        // Evaluate whether we want to print a time-warning
        bool show_time_warning = false;
        if (!status.client_statuses_.empty())
        {
          eCAL::Time::ecal_clock::duration min_time_error(eCAL::Time::ecal_clock::duration::max());
          eCAL::Time::ecal_clock::duration max_time_error(eCAL::Time::ecal_clock::duration::min());

          for (const auto& client_status : status.client_statuses_)
          {
            auto time_error = client_status.second.second - client_status.second.first.timestamp_;

            min_time_error = std::min(min_time_error, time_error);
            max_time_error = std::max(max_time_error, time_error);
          }

          show_time_warning = ((max_time_error - min_time_error) > std::chrono::milliseconds(500))
            || ((max_time_error - min_time_error) < std::chrono::milliseconds(-500));
        }


        // Iterate over clients that are connected and have a status
        for (const auto& client_status : status.client_statuses_)
        {
          // ========== Status of this Client ==========

          {
            std::vector<table_printer::TableEntry> table_row((int)Column::COLUMN_COUNT);

            // HOSTNAME
            eCAL::rec_cli::table_printer::TableEntry hostname_entry(client_status.first);

            // TIME
            eCAL::rec_cli::table_printer::TableEntry time_entry;
            auto time_as_time_t = std::chrono::system_clock::to_time_t(
                                        std::chrono::system_clock::time_point(
                                          std::chrono::duration_cast<std::chrono::system_clock::duration>(
                                            client_status.second.first.timestamp_.time_since_epoch()
                                            )
                                        )
                                      );

            std::stringstream time_ss;
            time_ss << std::put_time(std::localtime(&time_as_time_t), "%F %T");
            int remaining_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(client_status.second.second.time_since_epoch()).count() % 1000;
            time_ss << "." << std::setfill('0') << std::setw(3) << std::right << remaining_milliseconds;

            time_entry.content = time_ss.str();

            if (show_time_warning)
            {
              time_entry.background_color = eCAL::rec_cli::table_printer::Color::YELLOW;
            }

            // BUFFER
            eCAL::rec_cli::table_printer::TableEntry buffer_entry;
            std::stringstream buffer_ss;
            buffer_ss << std::fixed << std::setprecision(1)
              << std::chrono::duration_cast<std::chrono::duration<double>>(client_status.second.first.pre_buffer_length_.second).count()
              << " s / " << client_status.second.first.pre_buffer_length_.first << " frames";
            buffer_entry.content = buffer_ss.str();

            // STATE
            eCAL::rec_cli::table_printer::TableEntry state_entry;
            if (!client_status.second.first.initialized_)
            {
              state_entry.content = "Not subscribing";
            }
            else
            {
              // Check all job states for a "Recording" State

              bool is_recording = false;

              for (const auto& job_status : client_status.second.first.job_statuses_)
              {
                if (job_status.state_ == eCAL::rec::JobState::Recording)
                {
                  is_recording = true;
                  break;
                }
              }

              if (is_recording)
              {
                state_entry.content    = "Recording";
                state_entry.text_color = eCAL::rec_cli::table_printer::Color::RED;
              }
              else
              {
                state_entry.content    = "Subscribing (" + std::to_string(client_status.second.first.subscribed_topics_.size()) + " Topics)";
                state_entry.text_color = eCAL::rec_cli::table_printer::Color::GREEN;
              }
            }

            // INFO
            eCAL::rec_cli::table_printer::TableEntry info_entry(client_status.second.first.info_.second);
            if (!(client_status.second.first.info_.first))
              info_entry.background_color = eCAL::rec_cli::table_printer::Color::RED;


            table_row[(int)Column::CLIENT_HOSTNAME] = hostname_entry;
            table_row[(int)Column::TIME]            = time_entry;
            table_row[(int)Column::BUFFER]          = buffer_entry;
            table_row[(int)Column::STATE]           = state_entry;
            table_row[(int)Column::INFO]            = info_entry;


            client_table.push_back(std::move(table_row));
          }

          // ========== Addons of this Client ==========
          for (const auto& addon_status : client_status.second.first.addon_statuses_)
          {
            // Check if this addon is enabled. if not, we don't display it, even though we have a displayable status. The user however would be confused, as he didn't want that addon to show up.
            {
              auto configured_client_it = config.enabled_clients_config_.find(client_status.first);
              if (configured_client_it == config.enabled_clients_config_.end())
                continue;

              auto configured_addon_it = configured_client_it->second.enabled_addons_.find(addon_status.addon_id_);
              if (configured_addon_it == configured_client_it->second.enabled_addons_.end())
                continue;
            }

            std::vector<table_printer::TableEntry> table_row((int)Column::COLUMN_COUNT);

            // NAME
            eCAL::rec_cli::table_printer::TableEntry name_entry(client_status.first + " (" + (addon_status.name_.empty() ? addon_status.addon_id_ : addon_status.name_) + ")");

            // BUFFER
            eCAL::rec_cli::table_printer::TableEntry buffer_entry(std::to_string(addon_status.pre_buffer_length_frame_count_) + " frames");

            // STATE
            eCAL::rec_cli::table_printer::TableEntry state_entry;
            if (!addon_status.initialized_)
            {
              state_entry.content = "Not initialized";
            }
            else
            {
              bool is_recording = false;

              for (const auto& job_status : client_status.second.first.job_statuses_)
              {
                for (const auto& rec_addon_job_status : job_status.rec_addon_statuses_)
                {
                  if (rec_addon_job_status.first == addon_status.addon_id_)
                  {
                    if (rec_addon_job_status.second.state_ == eCAL::rec::RecAddonJobStatus::State::Recording)
                    {
                      is_recording = true;
                      goto end_job_status_loop;
                    }
                  }
                }
              }

            end_job_status_loop:
              
              if (is_recording)
              {
                state_entry.content    = "Recording";
                state_entry.text_color = eCAL::rec_cli::table_printer::Color::RED;
              }
              else
              {
                state_entry.content    = "Initialized";
                state_entry.text_color = eCAL::rec_cli::table_printer::Color::GREEN;
              }
            }

            // INFO
            eCAL::rec_cli::table_printer::TableEntry info_entry(addon_status.info_.second);
            if (!addon_status.info_.first)
              info_entry.background_color = eCAL::rec_cli::table_printer::Color::RED;

            table_row[(int)Column::CLIENT_HOSTNAME] = name_entry;
            table_row[(int)Column::BUFFER]          = buffer_entry;
            table_row[(int)Column::STATE]           = state_entry;
            table_row[(int)Column::INFO]            = info_entry;
            
            client_table.push_back(std::move(table_row));
          }

          // Addons of this client that are configured but not connected
          {
            auto configured_client_it = config.enabled_clients_config_.find(client_status.first);
            if (configured_client_it != config.enabled_clients_config_.end())
            {
              for (const auto& configured_addon_id : configured_client_it->second.enabled_addons_)
              {
                // Check if the configured addon has a status
                auto existing_addon_status_it = std::find_if(client_status.second.first.addon_statuses_.begin(), client_status.second.first.addon_statuses_.end()
                  , [&configured_addon_id](const eCAL::rec::RecorderAddonStatus& addon_status) -> bool { return addon_status.addon_id_ == configured_addon_id; });

                if (existing_addon_status_it != client_status.second.first.addon_statuses_.end())
                  continue;

                std::vector<table_printer::TableEntry> table_row((int)Column::COLUMN_COUNT);

                table_row[(int)Column::CLIENT_HOSTNAME] = table_printer::TableEntry(client_status.first + " (" + configured_addon_id + ")", table_printer::FormatFlag::CROSSED);
                table_row[(int)Column::STATE]           = table_printer::TableEntry("Offline", eCAL::rec_cli::table_printer::Color::DEFAULT, eCAL::rec_cli::table_printer::Color::RED);
                table_row[(int)Column::INFO]            = table_printer::TableEntry("Not connected", eCAL::rec_cli::table_printer::Color::DEFAULT, eCAL::rec_cli::table_printer::Color::RED);

                client_table.push_back(table_row);
              }
            }
          }
        }

        // Iterate over the configuration an look for clients that are not connected
        for (const auto& client_config : config.enabled_clients_config_)
        {
          // Skip existing clients
          auto existing_status_it = status.client_statuses_.find(client_config.first);
          if (existing_status_it != status.client_statuses_.end())
            continue;

          // Client
          {
            std::vector<table_printer::TableEntry> table_row((int)Column::COLUMN_COUNT);

            table_row[(int)Column::CLIENT_HOSTNAME] = table_printer::TableEntry(client_config.first, table_printer::FormatFlag::CROSSED);
            table_row[(int)Column::STATE]           = table_printer::TableEntry("Offline", eCAL::rec_cli::table_printer::Color::DEFAULT, eCAL::rec_cli::table_printer::Color::RED);
            table_row[(int)Column::INFO]            = table_printer::TableEntry("Not connected", eCAL::rec_cli::table_printer::Color::DEFAULT, eCAL::rec_cli::table_printer::Color::RED);

            client_table.push_back(table_row);
          }

          // Addons of client
          for (const auto& enabled_addon : client_config.second.enabled_addons_)
          {
            std::vector<table_printer::TableEntry> table_row((int)Column::COLUMN_COUNT);

            table_row[(int)Column::CLIENT_HOSTNAME] = table_printer::TableEntry(client_config.first + " (" + enabled_addon + ")", table_printer::FormatFlag::CROSSED);
            table_row[(int)Column::STATE]           = table_printer::TableEntry("Offline", eCAL::rec_cli::table_printer::Color::DEFAULT, eCAL::rec_cli::table_printer::Color::RED);
            table_row[(int)Column::INFO]            = table_printer::TableEntry("Not connected", eCAL::rec_cli::table_printer::Color::DEFAULT, eCAL::rec_cli::table_printer::Color::RED);

            client_table.push_back(table_row);
          }
        }

        table_printer::printTable(client_table, ostream);

        if (show_time_warning)
        {
          ostream << "\n";
          ostream << termcolor::on_yellow << "WARNING: The timestamps appear out of sync!" << termcolor::reset << "\n";
        }
      }


      eCAL::rec::Error Status::printJobState(const std::string& job_id_string, const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const
      {
        int64_t job_id;
        try
        {
          job_id = std::stoll(job_id_string.c_str());
        }
        catch (const std::exception& e)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Unable to parse ID " + job_id_string + ": " + std::string(e.what()));
        }
        
        return printJobState(job_id, status, ostream);
      }

      eCAL::rec::Error Status::printJobState(int64_t job_id, const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const
      {
        for (const auto& job_history_entry : status.job_history_)
        {
          if (job_id == job_history_entry.local_evaluated_job_config_.GetJobId())
            return printJobState(job_history_entry, status, ostream);
        }
        return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::MEAS_ID_NOT_FOUND, std::to_string(job_id));
      }

      eCAL::rec::Error Status::printJobState(const eCAL::rec_server::JobHistoryEntry& job_history_entry, const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const
      {
        // Main Information
        {
          // Length String
          const auto combined_length = combinedLength(job_history_entry);
          std::stringstream length_ss;
          length_ss << std::fixed << std::setprecision(1) << std::chrono::duration_cast<std::chrono::duration<double>>(combined_length.first).count() << " s / " << combined_length.second << " frames";

          // State string
          auto combined_state = combinedJobState(job_history_entry);
          std::string state_string = combinedJobStateToString(combined_state, job_history_entry);

          // Time string
          std::stringstream time_ss;
          auto time_as_time_t = std::chrono::system_clock::to_time_t(job_history_entry.local_start_time_);
          time_ss << std::put_time(std::localtime(&time_as_time_t), "%F %T");
          int remaining_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(job_history_entry.local_start_time_.time_since_epoch()).count() % 1000;
          time_ss << "." << std::setfill('0') << std::setw(3) << std::right << remaining_milliseconds;

          // Output data
          ostream << "Timestamp (loc.): " << time_ss.str() << "\n";
          ostream << "Name:             " << job_history_entry.local_evaluated_job_config_.GetMeasName() << "\n";
          ostream << "ID:               " << job_history_entry.local_evaluated_job_config_.GetJobId() << "\n";

          ostream << "Status:           ";
          if (combined_state == eCAL::rec::JobState::Recording)
            ostream << termcolor::red;
          ostream << state_string << termcolor::reset << "\n";

          ostream << "Local meas path:  " << job_history_entry.local_evaluated_job_config_.GetCompleteMeasurementPath() << "\n";
          ostream << "HDF5 Split size:  " << job_history_entry.local_evaluated_job_config_.GetMaxFileSize() << " MiB" << "\n";
          ostream << "Length:           " << length_ss.str() << "\n";
          ostream << "Uploaded:         " << (job_history_entry.is_uploaded_ ? "Yes" : "No") << "\n";

          ostream << "\n";
        }

        // Table for recorders that participated in the measurement
        enum class Column : int
        {
          NAME,
          PROCESS_ID,
          STATUS,
          LENGTH,
          INFO,
          COLUMN_COUNT
        };

        std::vector<std::vector<eCAL::rec_cli::table_printer::TableEntry>> recorder_table;
        recorder_table.reserve(job_history_entry.client_statuses_.size() + 1);

        {
          std::vector<table_printer::TableEntry> header_data((int)Column::COLUMN_COUNT);
          header_data[(int)Column::NAME]   = table_printer::TableEntry("Recorder");
          header_data[(int)Column::PROCESS_ID]    = table_printer::TableEntry("PROCESS_ID");
          header_data[(int)Column::STATUS] = table_printer::TableEntry("Status");
          header_data[(int)Column::LENGTH] = table_printer::TableEntry("Length");
          header_data[(int)Column::INFO]   = table_printer::TableEntry("Info");

          recorder_table.push_back(std::move(header_data));
        }

        for (const auto& client_status : job_history_entry.client_statuses_)
        {

          // Main recorder Status
          {
            std::vector<table_printer::TableEntry> table_row((int)Column::COLUMN_COUNT);

            // Length String
            std::stringstream length_ss;
            length_ss << std::fixed << std::setprecision(1) << std::chrono::duration_cast<std::chrono::duration<double>>(client_status.second.job_status_.rec_hdf5_status_.total_length_).count() << " s / " << client_status.second.job_status_.rec_hdf5_status_.total_frame_count_ << " frames";

            // State entry
            eCAL::rec_cli::table_printer::TableEntry rec_state_entry;
            if (job_history_entry.is_deleted_)
            {
              rec_state_entry.content = "Deleted";
            }
            else
            {
              switch (client_status.second.job_status_.state_)
              {
              case eCAL::rec::JobState::NotStarted:
                rec_state_entry.content = "Not Started";
                break;
              case eCAL::rec::JobState::Recording:
                rec_state_entry.content    = "Recording";
                rec_state_entry.text_color = table_printer::Color::RED;
                break;
              case eCAL::rec::JobState::Flushing:
                rec_state_entry.content = "Flushing (" + std::to_string(client_status.second.job_status_.rec_hdf5_status_.unflushed_frame_count_) + " frames)";
                break;
              case eCAL::rec::JobState::FinishedFlushing:
                rec_state_entry.content = "Finished Flushing";
                break;
              case eCAL::rec::JobState::Uploading:
                rec_state_entry.content = "Uploading (" + bytesToPrettyString(client_status.second.job_status_.upload_status_.bytes_uploaded_) + " of " + bytesToPrettyString(client_status.second.job_status_.upload_status_.bytes_total_size_) + ")";
                break;
              case eCAL::rec::JobState::FinishedUploading:
                rec_state_entry.content = "Finished Uploading";
                break;
              default:
                break;
              }
            }

            table_row[(int)Column::NAME]      .content = client_status.first;
            table_row[(int)Column::PROCESS_ID].content = std::to_string(client_status.second.client_pid_);
            table_row[(int)Column::STATUS]             = rec_state_entry;
            table_row[(int)Column::LENGTH]    .content = length_ss.str();

            auto displayed_info = displayedInfo(client_status.second);
            table_row[(int)Column::INFO]  .content = displayed_info.second;

            if (!displayed_info.first)
              table_row[(int)Column::INFO].background_color = eCAL::rec_cli::table_printer::Color::RED;

            if (job_history_entry.is_deleted_)
            {
              for (int i = 0; i < (int)Column::COLUMN_COUNT; i++)
              {
                table_row[i].format_flags |= table_printer::FormatFlag::CROSSED; // Unix only, Windows does not support "crossed" formatting
              }
            }

            recorder_table.push_back(table_row);
          }

          // Addon Statuses
          for (const auto& addon_status : client_status.second.job_status_.rec_addon_statuses_)
          {
            std::vector<table_printer::TableEntry> table_row((int)Column::COLUMN_COUNT);

            // State entry
            eCAL::rec_cli::table_printer::TableEntry rec_state_entry;
            if (job_history_entry.is_deleted_)
            {
              rec_state_entry.content = "Deleted";
            }
            else
            {
              switch (addon_status.second.state_)
              {
              case eCAL::rec::RecAddonJobStatus::NotStarted:
                rec_state_entry.content = "Not Started";
                break;
              case eCAL::rec::RecAddonJobStatus::Recording:
                rec_state_entry.content    = "Recording";
                rec_state_entry.text_color = table_printer::Color::RED;
                break;
              case eCAL::rec::RecAddonJobStatus::Flushing:
              {
                rec_state_entry.content = "Flushing (" + std::to_string(addon_status.second.unflushed_frame_count_) + " frames)";
              }
              break;
              case eCAL::rec::RecAddonJobStatus::FinishedFlushing:
                rec_state_entry.content = "Finished Flushing";
                break;
              default:
                break;
              }
            }

            table_row[(int)Column::NAME]      .content = getHumanReadableAddonName(client_status.first, addon_status.first, status);
            table_row[(int)Column::PROCESS_ID].content = std::to_string(client_status.second.client_pid_);
            table_row[(int)Column::STATUS]             = rec_state_entry;
            table_row[(int)Column::LENGTH]    .content = std::to_string(addon_status.second.total_frame_count_) + " frames";
            table_row[(int)Column::INFO]      .content = addon_status.second.info_.second;

            if (!addon_status.second.info_.first)
              table_row[(int)Column::INFO].background_color = eCAL::rec_cli::table_printer::Color::RED;

            if (job_history_entry.is_deleted_)
            {
              for (int i = 0; i < (int)Column::COLUMN_COUNT; i++)
              {
                table_row[i].format_flags |= table_printer::FormatFlag::CROSSED; // Unix only, Windows does not support "crossed" formatting
              }
            }

            recorder_table.push_back(table_row);
          }
        }

        ostream << "\n";
        eCAL::rec_cli::table_printer::printTable(recorder_table, ostream);
        
        return eCAL::rec::Error::ErrorCode::OK;
      }


      eCAL::rec::Error Status::printClientInformation(const std::string& hostname, const eCAL::rec_server::RecServerStatus& status, std::ostream& ostream) const
      {
          auto client_status_it = status.client_statuses_.find(hostname);
          if (client_status_it == status.client_statuses_.end())
            return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::CLIENT_UNKNOWN, hostname);

        {
          auto& client_status                  = client_status_it->second.first;
          auto  client_status_server_timestamp = client_status_it->second.second;

          std::chrono::duration<double> time_error(client_status_server_timestamp - client_status.timestamp_);
          std::stringstream time_error_ss;
          time_error_ss  << std::fixed << std::setprecision(3) << time_error.count();

          std::stringstream buffer_ss;
          buffer_ss << std::fixed << std::setprecision(1)
            << std::chrono::duration_cast<std::chrono::duration<double>>(client_status.pre_buffer_length_.second).count()
            << " s / " << client_status.pre_buffer_length_.first << " frames";

          ostream << "Client hostname: " << hostname << "\n";
          ostream << "Process Id:      " << client_status.pid_ << "\n";
          ostream << "Timestamp:       " << timeToString(client_status.timestamp_) << "\n";
          ostream << "Time-error:      " << time_error_ss.str() << " s" << "\n";
          ostream << "Buffer:          " << buffer_ss.str() << "\n";

          auto rec_client_state_eval = recClientStateToString(client_status);
          ostream << "State:           ";
          if (rec_client_state_eval.second == eCAL::rec::JobState::Recording)
            ostream << termcolor::red;
          ostream << rec_client_state_eval.first << termcolor::reset << "\n";

          ostream << "Info:            ";
          if (!client_status.info_.first)
            ostream << termcolor::on_red;
          ostream << client_status.info_.second << termcolor::reset << "\n";

          ostream << "Subscribed topics: " << "\n";
          if (client_status.subscribed_topics_.empty())
          {
            ostream << "  -- None --" << "\n";
          }
          else
          {
            for (const std::string& topic_name : client_status.subscribed_topics_)
            {
              ostream << "  " << topic_name << "\n";
            }
          }
        }

        // Addon information
        for (const auto& addon_status : client_status_it->second.first.addon_statuses_)
        {
          ostream << "\n"; 

          ostream << "Addon Name:      " << addon_status.name_ << "\n";
          ostream << "Addon ID:        " << addon_status.addon_id_ << "\n";
          ostream << "Addon State:     " << (addon_status.initialized_ ? "Initialized" : "Not initialized") << "\n";
          ostream << "Addon exe path:  " << addon_status.addon_executable_path_ << "\n";
          ostream << "Addon Buffer:    " << std::to_string(addon_status.pre_buffer_length_frame_count_) + " frames" << "\n";
          ostream << "Addon Info:      ";
          if (!addon_status.info_.first)
            ostream << termcolor::on_red;
          ostream << addon_status.info_.second << termcolor::reset << "\n";
        }

        return eCAL::rec::Error::ErrorCode::OK;
      }

      //////////////////////////////////////////////
      /// Job History Entry evaluation methods
      //////////////////////////////////////////////

      eCAL::rec::JobState Status::combinedJobState(const eCAL::rec_server::JobHistoryEntry& job_history_entry) const
      {
        // Create a map to count how many children have which state
        std::map<eCAL::rec::JobState, int> job_state_count
        {
          { eCAL::rec::JobState::NotStarted,        0 },
          { eCAL::rec::JobState::Recording,         0 },
          { eCAL::rec::JobState::Flushing,          0 },
          { eCAL::rec::JobState::FinishedFlushing,  0 },
          { eCAL::rec::JobState::Uploading,         0 },
          { eCAL::rec::JobState::FinishedUploading, 0 },
        };

        // Count Job States
        for (const auto& client_job_status : job_history_entry.client_statuses_)
        {
          job_state_count[client_job_status.second.job_status_.state_]++;

          // Count addon job states
          for (const auto& rec_addon_status : client_job_status.second.job_status_.rec_addon_statuses_)
          {
            switch (rec_addon_status.second.state_)
            {
            case eCAL::rec::RecAddonJobStatus::State::Recording:
              job_state_count[eCAL::rec::JobState::Recording]++;
              break;
            case eCAL::rec::RecAddonJobStatus::State::Flushing:
              job_state_count[eCAL::rec::JobState::Flushing]++;
              break;
            case eCAL::rec::RecAddonJobStatus::State::FinishedFlushing:
              job_state_count[eCAL::rec::JobState::FinishedFlushing]++;
              break;
            default:
              job_state_count[eCAL::rec::JobState::NotStarted]++;
            }
          }
        }

        // Evaluate counted Job states to calculate a combined state
        if (job_state_count[eCAL::rec::JobState::Recording] > 0)
        {
          return eCAL::rec::JobState::Recording;
        }
        else if (job_state_count[eCAL::rec::JobState::Flushing] > 0)
        {
          return eCAL::rec::JobState::Flushing;
        }
        else if ((job_state_count[eCAL::rec::JobState::FinishedFlushing] > 0)
          && (job_state_count[eCAL::rec::JobState::Uploading] == 0)
          && (job_state_count[eCAL::rec::JobState::FinishedUploading] == 0))
        {
          return eCAL::rec::JobState::FinishedFlushing;
        }
        else if (job_state_count[eCAL::rec::JobState::Uploading] > 0)
        {
          return eCAL::rec::JobState::Uploading;
        }
        else if (job_state_count[eCAL::rec::JobState::FinishedUploading] > 0)
        {
          return eCAL::rec::JobState::FinishedUploading;
        }
        else
        {
          return eCAL::rec::JobState::NotStarted;
        }
      }

      std::pair<std::chrono::steady_clock::duration, int64_t> Status::combinedLength(const eCAL::rec_server::JobHistoryEntry& job_history_entry) const
      {
        std::chrono::steady_clock::duration duration(0);
        int64_t                             frame_count(0);

        for (const auto& client_job_status : job_history_entry.client_statuses_)
        {
          duration     = std::max(duration, client_job_status.second.job_status_.rec_hdf5_status_.total_length_);
          frame_count += client_job_status.second.job_status_.rec_hdf5_status_.total_frame_count_;

          // Addons
          for (const auto& rec_addon_status : client_job_status.second.job_status_.rec_addon_statuses_)
          {
            frame_count += rec_addon_status.second.total_frame_count_;
          }
        }

        return { duration, frame_count };
      }

      int64_t Status::combinedUnflushedFrames(const eCAL::rec_server::JobHistoryEntry& job_history_entry) const
      {
        int64_t frame_count(0);

        for (const auto& client_job_status : job_history_entry.client_statuses_)
        {
          frame_count += client_job_status.second.job_status_.rec_hdf5_status_.unflushed_frame_count_;

          // Addons
          for (const auto& rec_addon_status : client_job_status.second.job_status_.rec_addon_statuses_)
          {
            frame_count += rec_addon_status.second.unflushed_frame_count_;
          }
        }

        return frame_count;
      }

      eCAL::rec::UploadStatus Status::combinedUploadStatus(const eCAL::rec_server::JobHistoryEntry& job_history_entry) const
      {
        eCAL::rec::UploadStatus status;

        for (const auto& client_job_status : job_history_entry.client_statuses_)
        {
          const eCAL::rec::UploadStatus& this_item_upload_status = client_job_status.second.job_status_.upload_status_;
          status.bytes_total_size_ += this_item_upload_status.bytes_total_size_;
          status.bytes_uploaded_   += this_item_upload_status.bytes_uploaded_;
        }

        return status;
      }

      std::pair<bool, std::string> Status::displayedInfo(const eCAL::rec_server::ClientJobStatus& client_job_status) const
      {
        // Error case
        if (!client_job_status.info_last_command_response_.first)
          return client_job_status.info_last_command_response_;
        else if (!client_job_status.job_status_.upload_status_.info_.first)
          return client_job_status.job_status_.upload_status_.info_;

        // No-error case
        else if (!client_job_status.info_last_command_response_.second.empty())
          return client_job_status.info_last_command_response_;
        else if (!client_job_status.job_status_.upload_status_.info_.second.empty())
          return client_job_status.job_status_.upload_status_.info_;
        else
          return {true, ""};
      }

      std::pair<bool, std::string> Status::combinedInfo(const eCAL::rec_server::JobHistoryEntry& job_history_entry, const eCAL::rec_server::RecServerStatus& status) const
      {
        std::map<std::string, std::pair<bool, std::string>> error_infos;

        for (const auto& client_job_status : job_history_entry.client_statuses_)
        {
          // Error of the main recorder
          auto displayed_info = displayedInfo(client_job_status.second);
          if (!displayed_info.first)
            error_infos.emplace(client_job_status.first, displayed_info);

          // Errors of addons
          for (const auto& rec_addon_statuses : client_job_status.second.job_status_.rec_addon_statuses_)
          {
            if (!rec_addon_statuses.second.info_.first)
            {
              error_infos.emplace(getHumanReadableAddonName(client_job_status.first, rec_addon_statuses.first, status), rec_addon_statuses.second.info_);
            }
          }
        }

        if (error_infos.empty())
        {
          // No Error: Just return no error
          return { true, "" };
        }
        else if (error_infos.size() == 1)
        {
          // Only 1 Error: Return the error description of that one error
          return { false, error_infos.begin()->first + ": " + error_infos.begin()->second.second };
        }
        else
        {
          // 2 or more errors: Return a list of all items that have an error
          std::string error_items_string = std::to_string(error_infos.size()) + " Errors: ";
          for (auto error_info_it = error_infos.begin(); error_info_it != error_infos.end(); error_info_it++)
          {
            if (error_info_it != error_infos.begin())
              error_items_string += std::string(", ");

            error_items_string += error_info_it->first;
          }

          return { false, error_items_string };
        }
      }

      std::string Status::getHumanReadableAddonName(const std::string& hostname, const std::string& addon_id, const eCAL::rec_server::RecServerStatus& status) const
      {
        auto host_it = status.client_statuses_.find(hostname);
        if (host_it != status.client_statuses_.end())
        {
          for (const auto& addon_status : host_it->second.first.addon_statuses_)
          {
            if (addon_status.addon_id_ == addon_id)
            {
              if (addon_status.name_ != "")
                return hostname + " (" + addon_status.name_ + ")";
              else
                break;
            }
          }
        }
        return hostname + " (" + addon_id + ")";
      }

      std::string Status::timeToString(eCAL::Time::ecal_clock::time_point time) const
      {
        return timeToString(std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(time.time_since_epoch())));
      }

      std::string Status::timeToString(std::chrono::system_clock::time_point time) const
      {
        auto time_as_time_t = std::chrono::system_clock::to_time_t(time);
        std::stringstream time_ss;
        time_ss << std::put_time(std::localtime(&time_as_time_t), "%F %T");
        int remaining_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count() % 1000;
        time_ss << "." << std::setfill('0') << std::setw(3) << std::right << remaining_milliseconds;
        return time_ss.str();
      }

      std::string Status::combinedJobStateToString(eCAL::rec::JobState combined_job_state, const eCAL::rec_server::JobHistoryEntry& job_history_entry) const
      {
        std::string state_string;

        if (job_history_entry.is_deleted_)
        {
          state_string = "Deleted";
        }
        else
        {
          switch (combined_job_state)
          {
          case eCAL::rec::JobState::NotStarted:
            state_string = "Not Started";
            break;
          case eCAL::rec::JobState::Recording:
            state_string    = "Recording";
            break;
          case eCAL::rec::JobState::Flushing:
          {
            // Count the number of frames to flush
            int64_t frames_to_flush(0);
            for (const auto& client_status : job_history_entry.client_statuses_)
            {
              frames_to_flush += client_status.second.job_status_.rec_hdf5_status_.unflushed_frame_count_;
              for (const auto& addon_status : client_status.second.job_status_.rec_addon_statuses_)
              {
                frames_to_flush += addon_status.second.unflushed_frame_count_;
              }
            }
            state_string = "Flushing (" + std::to_string(frames_to_flush) + " frames)";
          }
          break;
          case eCAL::rec::JobState::FinishedFlushing:
            state_string = "Finished Flushing";
            break;
          case eCAL::rec::JobState::Uploading:
          {
            // Count bytes to upload
            uint64_t bytes_uploaded         (0);
            uint64_t total_bytes_to_uploaded(0);
            for (const auto& client_status : job_history_entry.client_statuses_)
            {
              if ((client_status.second.job_status_.state_ == eCAL::rec::JobState::Uploading)
                || (client_status.second.job_status_.state_ == eCAL::rec::JobState::FinishedUploading))
              {
                total_bytes_to_uploaded += client_status.second.job_status_.upload_status_.bytes_total_size_;
                bytes_uploaded          += client_status.second.job_status_.upload_status_.bytes_uploaded_;
              }
            }
            state_string = "Uploading (" + bytesToPrettyString(bytes_uploaded) + " of " + bytesToPrettyString(total_bytes_to_uploaded) + ")";
          }
            break;
          case eCAL::rec::JobState::FinishedUploading:
            state_string = "Finished Uploading";
            break;
          default:
            break;
          }
        }

        return state_string;
      }

      std::pair<std::string, eCAL::rec::JobState> Status::recClientStateToString(eCAL::rec::RecorderStatus rec_client_status) const
      {
        if (!rec_client_status.initialized_)
        {
          return {"Not subscribing", eCAL::rec::JobState::NotStarted};
        }
        else
        {
          // Recording
          for (const auto& job_status : rec_client_status.job_statuses_)
          {
            if (job_status.state_ == eCAL::rec::JobState::Recording)
            {
              return {"Recording", eCAL::rec::JobState::Recording};
            }
          }

          
          // Subscribing
          return {std::string("Subscribing (") + std::to_string(rec_client_status.subscribed_topics_.size()) + " Topics)",  eCAL::rec::JobState::NotStarted};
        }
      }
    }
  }
}
