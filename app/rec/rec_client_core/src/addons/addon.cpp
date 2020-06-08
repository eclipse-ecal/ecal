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

#include "addon.h"

#include "function_descriptors.h"

#include <rec_client_core/ecal_rec_logger.h>
#include <ecal_utils/filesystem.h>

using namespace eCAL::rec::addon;

namespace eCAL
{
  namespace rec
  {
    ////////////////////////////////////////////
    // Constructor & destructor
    ////////////////////////////////////////////
    Addon::Addon(const std::string& executable_path, std::function<void(std::int64_t job_id, const std::string& addon_id, const RecAddonJobStatus& job_status)> set_job_status_function)
      : status_thread_enabled_{false}, pre_buffering_enabled_{false}, currently_recording_job_id_(0), set_job_status_function_(set_job_status_function)
    {
      response_handler_.SetFunctionDescriptors(
      {
        function_descriptor::api_version,
        function_descriptor::deinitialize,
        function_descriptor::disable_prebuffering,
        function_descriptor::enable_prebuffering,
        function_descriptor::info,
        function_descriptor::initialize,
        function_descriptor::job_statuses,
        function_descriptor::prebuffer_count,
        function_descriptor::save_prebuffer,
        function_descriptor::set_prebuffer_length,
        function_descriptor::start_recording,
        function_descriptor::stop_recording
      });

      last_status_.addon_executable_path_ = executable_path;

      if (!pipe_handler_.StartProcess(executable_path))
      {
        last_status_.info_.first = false;
        last_status_.info_.second = "Unable to start recorder add-on " + executable_path + ".";
        return;
      }
      
      response_handler_.SetWriteLineCallback([this](const std::string& line) -> bool
      {
//#ifndef NDEBUG
//        eCAL::rec::EcalRecLogger::Instance()->debug("Request line: " + line);
//#endif
        return pipe_handler_.WriteLine(line);
      });

      response_handler_.SetReadLineCallback([this](std::string& line) -> bool
      {
        line = pipe_handler_.ReadLine();
//#ifndef NDEBUG
//        eCAL::rec::EcalRecLogger::Instance()->debug("Response line: " + line);
//#endif
        return true;
      });
      
      request_queue_.SetFlushingCallback([this](const auto& request_callback_pair)
      {
        if(!this->IsRunning())
          return;

        addon::Response response;
        if (!response_handler_.Query(request_callback_pair.first, response))
        {
          eCAL::rec::EcalRecLogger::Instance()->warn("Reponse handler error: " + response_handler_.GetErrorString());
          return;
        }

        {
          std::lock_guard<std::mutex> lock(status_mutex_);
          if (!response.status_message.empty())
          {
            last_status_.info_.second = response.status_message;
            last_status_.info_.first = (response.status == addon::Response::Status::Ok) ? true : false;
          }
        }

        if (response.status == addon::Response::Status::Ok)
        {
          request_callback_pair.second(response);
        }
        else
        {
          eCAL::rec::EcalRecLogger::Instance()->warn("Response error: " + response.status_message);
        }

      });

      request_queue_.Add(std::make_pair(
        addon::Request
      { 
        "info",
        {} 
      },
        [this](const addon::Response& response)
      {
        std::lock_guard<std::mutex> lock(status_mutex_);
        last_status_.addon_id_ = response.results.front().at("id").GetStringValue();
        last_status_.name_ = response.results.front().at("name").GetStringValue();

        std::thread status_thread([this]() {
          status_thread_enabled_ = true;
          while (status_thread_enabled_)
          {
            // Prevents spamming the request queue from status queries
            if(request_queue_.Count() > 100) continue;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            request_queue_.Add(std::make_pair(
              Request
            {
              "prebuffer_count",
              {}
            },
              [this](const Response& response)
            {
              std::lock_guard<std::mutex> lock(status_mutex_);
              last_status_.pre_buffer_length_frame_count_ = response.results.front().at("frame_count").GetIntegerValue();
            }));

            request_queue_.Add(std::make_pair(
              Request
            {
              "job_statuses",
              {}
            },
              [this](const Response& response)
            {
              std::vector<std::pair<std::int64_t,RecAddonJobStatus>> job_status_pairs;

              for (const auto& result : response.results)
              {
                std::pair<std::int64_t, RecAddonJobStatus> job_status;
                
                static const std::unordered_map<std::string, RecAddonJobStatus::State> job_statuses_map
                {
                  { "not started", RecAddonJobStatus::State::NotStarted },
                  { "recording", RecAddonJobStatus::State::Recording },
                  { "flushing", RecAddonJobStatus::State::Flushing },
                  { "finished", RecAddonJobStatus::State::FinishedFlushing }
                };

                job_status.second.state_ = job_statuses_map.at(result.at("state").GetStringValue());
                job_status.second.total_frame_count_ = result.at("frame_count").GetIntegerValue();
                job_status.second.unflushed_frame_count_ = result.at("queue_count").GetIntegerValue();
                job_status.second.info_.first = result.at("healthy").GetBooleanValue();
                job_status.second.info_.second = result.at("status_description").GetStringValue();
                job_status.first = result.at("id").GetIntegerValue();

                job_status_pairs.push_back(std::move(job_status));
              }

              for (const auto& job_status_pair : job_status_pairs)
              {
                set_job_status_function_(job_status_pair.first, last_status_.addon_id_, job_status_pair.second);
              }

            }));
          }
        });
        std::swap(status_thread, status_thread_);
      }));

      request_queue_.EnableFlushing();
    }

    Addon::~Addon()
    {
      request_queue_.DisableFlushing();

      if (status_thread_enabled_)
      {
        status_thread_enabled_ = false;
        status_thread_.join();

        // send EOF for add-on termination
        pipe_handler_.WriteLine("\x1c");
      }
      
      // force add-on process termination
      pipe_handler_.StopProcess();
    }

    ////////////////////////////////////////////
    // public API
    ////////////////////////////////////////////
    void Addon::Initialize()
    {
      request_queue_.Add(std::make_pair(
        addon::Request{ "initialize",{} },
        [this](const addon::Response&)
      {
        std::lock_guard<std::mutex> lock(status_mutex_);
        last_status_.initialized_ = true;
      }));
    }

    void Addon::Deinitialize()
    {
      request_queue_.Add(std::make_pair(
        addon::Request
      { 
        "deinitialize",
        {} 
      },
        [this](const addon::Response&)
      {
        std::lock_guard<std::mutex> lock(status_mutex_);
        last_status_.initialized_ = false;
      }));
    }

    void Addon::SetPreBuffer(bool enabled, std::chrono::steady_clock::duration length)
    {
      request_queue_.Add(std::make_pair(
        addon::Request
      {
        "set_prebuffer_length",
        {
          { "duration", addon::Variant(static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(length).count())) }
        }
      },
        [](const auto&)
      {
      }));

      if (enabled)
      {
        if (!pre_buffering_enabled_)
        {
          request_queue_.Add(std::make_pair(
            addon::Request
          {
            "enable_prebuffering",
            {}
          },
            [this](const auto&)
          {
            pre_buffering_enabled_ = true;
          }));
        }
      }
      else
      {
        request_queue_.Add(std::make_pair(
          addon::Request
        { 
          "disable_prebuffering",
          {} 
        },
          [this](const auto&)
        {
          pre_buffering_enabled_ = false;
        }));
      }
    }

    void Addon::StartRecording(const JobConfig& job_config)
    {
      if (currently_recording_job_id_ == 0)
      {
        request_queue_.Add(std::make_pair(
          addon::Request
        {
          "start_recording",
          {
            { "id", Variant(job_config.GetJobId()) },
            { "path", Variant(job_config.GetCompleteMeasurementPath() + std::string(1, EcalUtils::Filesystem::NativeSeparator()) + eCAL::Process::GetHostName()) }
          }
        },
          [this, job_config](const auto&)
        {
          if (currently_recording_job_id_ == 0)
          {
            currently_recording_job_id_ = job_config.GetJobId();
          }
        }));
      }
    }

    void Addon::StopRecording()
    {
      if (currently_recording_job_id_ != 0)
      {
        request_queue_.Add(std::make_pair(
          addon::Request
        {
          "stop_recording",
          {
            { "id", Variant(static_cast<std::int64_t>(currently_recording_job_id_)) },
          }
        },
          [this](const auto&)
        {
            currently_recording_job_id_ = 0;
        }));
      }
    }

    void Addon::SaveBuffer(const JobConfig& job_config)
    {
      request_queue_.Add(std::make_pair(
        addon::Request
      {
        "save_prebuffer",
        {
          { "id", Variant(job_config.GetJobId()) },
          { "path", Variant(job_config.GetCompleteMeasurementPath() + std::string(1, EcalUtils::Filesystem::NativeSeparator()) + eCAL::Process::GetHostName()) }
        }
      },
        [job_config](const auto&)
      {
      }));
    }

    std::string Addon::GetAddonId() const
    {
      std::lock_guard<std::mutex> status_lock(status_mutex_);
      return last_status_.addon_id_;
    }

    RecorderAddonStatus Addon::GetStatus() const
    {
      std::lock_guard<std::mutex> status_lock(status_mutex_);
      return last_status_;
    }

    bool Addon::IsRunning() const
    {
      return pipe_handler_.IsProcessAlive();
    }
  }
}
