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

#include "play_thread.h"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cfloat>

#include <ecal/ecal.h>

#include "state_publisher_thread.h"
#include "ecal_play_logger.h"

PlayThread::PlayThread()
  : time_log_complete_time_span_(0)
{
  state_publisher_thread_ = std::unique_ptr<StatePublisherThread>(new StatePublisherThread(*this));
  state_publisher_thread_->Start();
}

PlayThread::~PlayThread()
{
  DeInitializePublishers();
}

void PlayThread::Interrupt()
{
  InterruptibleThread::Interrupt();
  pause_cv_.notify_all();

  state_publisher_thread_->Interrupt();
  state_publisher_thread_->Join();
}

void PlayThread::Run()
{
  while (!IsInterrupted())
  {
    EcalPlayCommand command;
    {
      std::unique_lock<std::mutex> command_lock(command_mutex_);

      // Wait until the playback is not paused any more and we have waited long enough 
      do
      {
        if (IsInterrupted()) return;

        //////////////////////////
        // IF PAUSED            //
        //////////////////////////
        if (!command_.playing_)
        {
          // Wait until the command changed to playing (or the thread is interrupted)
          pause_cv_.wait(command_lock, [this]() {return IsInterrupted() || command_.playing_; }); //-V560
          if (IsInterrupted()) return;
        }


        //////////////////////////
        // IF PLAYING           //
        //////////////////////////
        if (command_.playing_)
        {
          if (command_.limit_play_speed_)
          {

            // If the last frame took too long, we set back the simtime
            if (command_.enforce_delay_accuracy_)
            {
              if ((GetCurrentSimTime_Private() - std::chrono::milliseconds(1)) > command_.next_frame_timestamp_)
              {
                SetSimTime_Private(command_.next_frame_timestamp_);
              }
            }

            // Wait until we reached the desired time (or the thread is interrupted)
            auto wait_until_timepoint = GetSystemTime_Private(command_.next_frame_timestamp_);
            while (command_.playing_ && (std::chrono::steady_clock::now() < wait_until_timepoint) && command_.limit_play_speed_)
            {
              if (IsInterrupted()) return;
              // While waiting, the play rate may change. In that case, the
              // waiting is interrupted, but must resum with a different time
              // point. Therefore the surrounding while-loop instead of a
              // condition variable predicate function.
              // When pausing while waiting for the next frame, the outer
              // do-while loop causes the 
#ifdef WIN32
              pause_cv_.wait_until(command_lock, wait_until_timepoint);
#else // WIN32
              if (wait_until_timepoint == std::chrono::steady_clock::time_point::max())
              {
                pause_cv_.wait(command_lock);
              }
              else
              {
                pause_cv_.wait_until(command_lock, wait_until_timepoint);
              }
#endif // WIN32
              wait_until_timepoint = GetSystemTime_Private(command_.next_frame_timestamp_);
            } 

            if (IsInterrupted()) return;
          }
          
          if (!command_.limit_play_speed_)
          {
            // When running with infinite speed, we don't wait, but simply step the sim time.
            SetSimTime_Private(command_.next_frame_timestamp_);
          }
        }
      } while (!command_.playing_ && !IsInterrupted());

      if (IsInterrupted()) return;

      // Thread-safe copy the command. This is the command we execute. We don't
      // care about any changes at this point. This improves performance, as
      // publishing frames may take some time and we don't want commands to 
      // be blocked during that time
      command_.next_frame_set_manually_ = false;
      command = command_;
    }

    std::pair<eCAL::Time::ecal_clock::time_point, std::chrono::nanoseconds> publish_time;

    {
      std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
      if (IsInterrupted()) return;

      // Publish the desired frame
      if (measurement_container_)
      {
        measurement_container_->PublishFrame(command.next_frame_index_);

        auto elapsed_time = frame_stopwatch_.GetElapsedTimeAndRestart();
        
        publish_time.first  = command.next_frame_timestamp_;
        publish_time.second = elapsed_time;

        // Calculate the next frame
        long long next_frame_index = command.next_frame_index_;
        do {
          // Re-compute the next frame if framedropping is enabled and the next frame would already be too late
          next_frame_index = measurement_container_->GetNextEnabledFrameIndex(next_frame_index, command.repeat_enabled_, command.limit_interval_);
        } while (command.framedropping_allowed_
              && command.limit_play_speed_
              && (next_frame_index > command.next_frame_index_)
              && (next_frame_index > 0)
              && measurement_container_->GetTimestamp(next_frame_index) < GetCurrentSimTime_Private());
       

        if (next_frame_index < 0)
        {
          // There is nothing to play any more, e.g. because we have reached the end
          long long zero_index = std::max(0LL, command.limit_interval_.first);
          command.playing_                    = false;
          command.current_frame_index_        = zero_index;
          command.current_frame_timestamp_    = measurement_container_->GetTimestamp(zero_index);
          command.next_frame_index_           = zero_index;
          command.next_frame_timestamp_       = measurement_container_->GetTimestamp(zero_index);
        }
        else
        {
          // We have more frames and just continue to the next one

          // Pause if we have reached the index that we wanted to reach
          if (command.play_until_index_ >= 0)
          {
            long long last_index = command.next_frame_index_;
            long long until_index = command.play_until_index_;
            long long next_index = next_frame_index;

            if (((last_index <= until_index) && (next_index > until_index))  // not looped
              || ((last_index <= until_index) && (next_index < last_index))) // looped
            {
              command.playing_ = false;
            }
          }

          // We have more frames and just continue to the next one
          command.current_frame_index_        = command.next_frame_index_;
          command.current_frame_timestamp_    = command.next_frame_timestamp_;
          command.next_frame_index_           = next_frame_index;
          command.next_frame_timestamp_       = measurement_container_->GetTimestamp(command.next_frame_index_);
        }
      }
    }

    {
      std::unique_lock<std::mutex> command_lock(command_mutex_);
      // We unlocked the command mutex while publishing the frame and computing
      // the next one. If someone set the next frame manually during that time,
      // we don't want to overwrite this information

      bool publish_state_necessary = false;

      // update the time log
      time_log_.push_back(publish_time);
      time_log_complete_time_span_ += publish_time.second;

      if (!command_.next_frame_set_manually_)
      {
        if (command.next_frame_index_ < command_.next_frame_index_)
        {
          // Reset the time log if we jumped backwards (i.e. have looped)
          time_log_.clear();
          time_log_complete_time_span_ = std::chrono::nanoseconds(0);
        }

        if (!command.playing_)
        {
          frame_stopwatch_.Pause();

          command_.playing_                    = false;
          command_.current_frame_index_        = command.current_frame_index_;
          command_.current_frame_timestamp_    = command.current_frame_timestamp_;
          command_.next_frame_index_           = command.next_frame_index_;
          command_.next_frame_timestamp_       = command.next_frame_timestamp_;

          SetSimTime_Private(command_.current_frame_timestamp_);

          publish_state_necessary = true;
        }
        else
        {
          if (command.next_frame_index_ < command_.next_frame_index_)
          {
            // Reset the Simtime to the first frame if we have looped
            SetSimTime_Private(command_.limit_interval_times_.first);
            publish_state_necessary = true;
          }

          command_.current_frame_index_        = command.current_frame_index_;
          command_.current_frame_timestamp_    = command.current_frame_timestamp_;
          command_.next_frame_index_           = command.next_frame_index_;
          command_.next_frame_timestamp_       = command.next_frame_timestamp_;

        }
      }

      // Limit the time log to 1 second
      while ((time_log_.size() > 2)
        && (time_log_complete_time_span_ - time_log_.front().second) >= std::chrono::seconds(1))
      {
        time_log_complete_time_span_ -= time_log_.front().second;
        time_log_.pop_front();
      }

      // Publish the new information as fast as possible
      if (publish_state_necessary)
      {
        state_publisher_thread_->PublishNow();
      }

      // Log
      if (!command.playing_)
      {
        EcalPlayLogger::Instance()->info("Playback finished");
        eCAL::Process::SetState(eCAL_Process_eSeverity::proc_sev_healthy, eCAL_Process_eSeverity_Level::proc_sev_level1, "Playback finished");
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//// Private Functions that are NOT THREAD SAFE                             ////
////////////////////////////////////////////////////////////////////////////////

void PlayThread::SetPlayRate_Private(double play_rate)
{
  if (command_.playing_)
  {
    // Time has only elapsed if the playback was running
    auto now = std::chrono::steady_clock::now();

    auto elapsed_sim_time = (now - command_.sim_time_local_timestamp_) * command_.play_speed_;
    auto current_sim_time = command_.sim_time_ + std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_sim_time);

    command_.sim_time_                 = current_sim_time;
    command_.sim_time_local_timestamp_ = now;
  }

  command_.play_speed_ = std::max(0.0, play_rate);

  // Publish the new information as fast as possible
  state_publisher_thread_->PublishNow();
}

void PlayThread::SetSimTime_Private(eCAL::Time::ecal_clock::time_point current_sim_time)
{
  command_.sim_time_local_timestamp_ = std::chrono::steady_clock::now();
  command_.sim_time_                 = current_sim_time;
}

eCAL::Time::ecal_clock::time_point PlayThread::GetCurrentSimTime_Private() const
{
  if (command_.playing_)
  {
    // Time has only elapsed if the playback was running
    auto now = std::chrono::steady_clock::now();

    auto elapsed_sim_time = (now - command_.sim_time_local_timestamp_) * command_.play_speed_;
    auto current_sim_time = command_.sim_time_ + std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_sim_time);

    return current_sim_time;
  }
  else
  {
    return command_.sim_time_;
  }
}

std::chrono::steady_clock::time_point PlayThread::GetSystemTime_Private(eCAL::Time::ecal_clock::time_point sim_time) const
{
  if (fabs(command_.play_speed_) < DBL_EPSILON)
  {
    // If the play rate is 0, we wait until the end of times
    return std::chrono::steady_clock::time_point::max();
  }

  auto scaled_sim_time_diff = (sim_time - command_.sim_time_) / command_.play_speed_;
  return command_.sim_time_local_timestamp_ + std::chrono::duration_cast<std::chrono::nanoseconds>(scaled_sim_time_diff);
}

void PlayThread::SetPlaying_Private(bool playing)
{
  if (command_.playing_ && !playing)
  {
    frame_stopwatch_.Pause();

    // Time has only elapsed if the playback was running
    auto now = std::chrono::steady_clock::now();

    auto elapsed_sim_time = (now - command_.sim_time_local_timestamp_) * command_.play_speed_;
    auto current_sim_time = command_.sim_time_ + std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_sim_time);

    command_.sim_time_                 = current_sim_time;
    command_.sim_time_local_timestamp_ = now;

    EcalPlayLogger::Instance()->info("Playback pause");
    eCAL::Process::SetState(eCAL_Process_eSeverity::proc_sev_healthy, eCAL_Process_eSeverity_Level::proc_sev_level1, "Paused");
  }
  else if (!command_.playing_ && playing)
  {
    frame_stopwatch_.Resume();

    // No time has elapsed, the existing simtime resumes
    command_.sim_time_local_timestamp_ = std::chrono::steady_clock::now();

    EcalPlayLogger::Instance()->info("Playback start");
    eCAL::Process::SetState(eCAL_Process_eSeverity::proc_sev_healthy, eCAL_Process_eSeverity_Level::proc_sev_level1, "Playing");
  }

  command_.playing_ = playing;

  // Publish the new information as fast as possible
  state_publisher_thread_->PublishNow();
}

double PlayThread::GetCurrentPlaySpeed_Private() const
{
  if (time_log_.size() < 2)
  {
    return command_.play_speed_;
  }
  else
  {
    std::chrono::duration<double> elapsed_system_time(0.0);
    std::chrono::duration<double> elapsed_sim_time(0.0);

    for (int i = (int)time_log_.size() - 1; i > 0; i--)
    {
      elapsed_sim_time += (time_log_[i].first - time_log_[i - 1].first);
      elapsed_system_time += time_log_[i].second;
    }

    return elapsed_sim_time / elapsed_system_time;
  }
}

bool PlayThread::InitializePublishers_Private(bool use_channel_mapping, const std::map<std::string, std::string>& channel_mapping)
{
  bool success = false;

  long long                       last_index;
  std::pair<long long, long long> limit_interval;
  bool                            repeat_enabled;

  long long next_frame_index(0);
  eCAL::Time::ecal_clock::time_point next_frame_timestamp;

  {
    std::lock_guard<std::mutex> command_lock(command_mutex_);
    last_index     = command_.current_frame_index_;
    limit_interval = command_.limit_interval_;
    repeat_enabled = command_.repeat_enabled_;
  }

  {
    std::unique_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
    if (measurement_container_)
    {
      if (use_channel_mapping)
      {
        EcalPlayLogger::Instance()->info("Initializing " + std::to_string(channel_mapping.size()) + " eCAL publishers");
        LogChannelMapping(channel_mapping);
        measurement_container_->CreatePublishers(channel_mapping);
      }
      else
      {
        EcalPlayLogger::Instance()->info("Initializing all eCAL publishers with default names");
        measurement_container_->CreatePublishers();
      }

      next_frame_index     = measurement_container_->GetNextEnabledFrameIndex(last_index - 1, repeat_enabled, limit_interval);
      next_frame_timestamp = measurement_container_->GetTimestamp(next_frame_index);

      success = true;
    }

  }

  {
    if (success)
    {
      std::lock_guard<std::mutex> command_lock(command_mutex_);
      command_.next_frame_index_     = next_frame_index;
      command_.next_frame_timestamp_ = next_frame_timestamp;
      pause_cv_.notify_all();
    }
  }

  return success;
}

bool PlayThread::SetLimitInterval_Private(const std::pair<long long, long long>* indexes, const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>* timestamps)
{
  if ((indexes == nullptr) && (timestamps == nullptr)) return false;

  long long lower_index(0);
  long long upper_index(0);

  eCAL::Time::ecal_clock::time_point lower_time;
  eCAL::Time::ecal_clock::time_point upper_time;

  {
    std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
    if (!measurement_container_ || (measurement_container_->GetFrameCount() == 0))
    {
      return false;
    }

    if ((indexes == nullptr) && timestamps)
    {
      // Convert the time to index values
      if (timestamps->first < timestamps->second)
      {
        lower_index = measurement_container_->GetNearestIndex(timestamps->first);
        upper_index = measurement_container_->GetNearestIndex(timestamps->second);
      }
      else
      {
        upper_index = measurement_container_->GetNearestIndex(timestamps->first);
        lower_index = measurement_container_->GetNearestIndex(timestamps->second);
      }
    }
    else
    {
      if (indexes)
      {
        lower_index = indexes->first;
        upper_index = indexes->second;
      }
    }

    // Limit the values to valid indices
    lower_index = std::max(0LL, lower_index);
    lower_index = std::min(lower_index, measurement_container_->GetFrameCount() - 1LL);

    upper_index = std::max(0LL, upper_index);
    upper_index = std::min(upper_index, measurement_container_->GetFrameCount() - 1LL);

    lower_time = measurement_container_->GetTimestamp(lower_index);
    upper_time = measurement_container_->GetTimestamp(upper_index);
  }

  EcalPlayLogger::Instance()->info("Setting limit interval to indexes: [" + std::to_string(lower_index) + ", " + std::to_string(upper_index) + "]");

  {
    // Update the command struct
    std::lock_guard<std::mutex> command_lock(command_mutex_);

    command_.limit_interval_.first  = lower_index;
    command_.limit_interval_.second = upper_index;

    command_.limit_interval_times_.first  = lower_time;
    command_.limit_interval_times_.second = upper_time;

    // Only set the current / next frame if necessary
    if ((command_.current_frame_index_ < lower_index)
      || (command_.next_frame_index_ < lower_index))
    {
      command_.current_frame_index_        = lower_index;
      command_.current_frame_timestamp_    = lower_time;
      command_.next_frame_index_           = lower_index;
      command_.next_frame_timestamp_       = lower_time;
      command_.next_frame_set_manually_    = true;
      SetSimTime_Private(lower_time);
      pause_cv_.notify_all();
    }
    else if ((command_.current_frame_index_ > upper_index)
      || (command_.next_frame_index_ > upper_index))
    {
      command_.current_frame_index_        = upper_index;
      command_.current_frame_timestamp_    = upper_time;
      command_.next_frame_index_           = upper_index;
      command_.next_frame_timestamp_       = upper_time;
      command_.next_frame_set_manually_    = true;
      SetSimTime_Private(upper_time);
      pause_cv_.notify_all();
    }
  }
  return true;
}

void PlayThread::LogChannelMapping(const std::map<std::string, std::string>& channel_mapping)
{

  size_t left_size = 0;
  for (const auto& entry : channel_mapping)
  {
    left_size = std::max(left_size, entry.first.size());
  }
  left_size = std::min(left_size, size_t(38));

  std::stringstream ss;
  ss << "Channel mapping:" << std::endl;
  if (channel_mapping.size() == 0)
  {
    ss << "  -- no channels --" << std::endl;
  }

  for (const auto& entry : channel_mapping)
  {
    if (entry.first != entry.second)
    {
      ss << "  " << std::left << std::setw(left_size) << entry.first;
      ss << " -> " << entry.second << std::endl;
    }
    else
    {
      ss << "  " << entry.first << std::endl;
    }
  }
  EcalPlayLogger::Instance()->info(ss.str());
}

////////////////////////////////////////////////////////////////////////////////
//// Measurement                                                            ////
////////////////////////////////////////////////////////////////////////////////

void PlayThread::SetMeasurement(std::shared_ptr<eCAL::eh5::HDF5Meas> measurement, const std::string& path)
{
  std::unique_ptr<MeasurementContainer> new_measurment_container;

  if (measurement)
  {
    new_measurment_container = std::unique_ptr<MeasurementContainer>(new MeasurementContainer(measurement, path));
  }

  {
    // Pause the playback and set Information about the new measurement
    std::unique_lock<std::mutex> command_lock(command_mutex_);

    command_.playing_                    = false;

    command_.current_frame_index_        = 0;
    command_.next_frame_index_           = 0;
    command_.next_frame_set_manually_    = true;

    if (new_measurment_container && (new_measurment_container->GetFrameCount() > 0))
    {
      command_.current_frame_timestamp_     = new_measurment_container->GetTimestamp(0);
      command_.next_frame_timestamp_        = new_measurment_container->GetTimestamp(0);
      command_.first_measurement_timestamp_ = new_measurment_container->GetTimestamp(0);
      command_.last_measurement_timestamp_  = new_measurment_container->GetTimestamp(new_measurment_container->GetFrameCount() - 1);

      command_.limit_interval_.first        = 0;
      command_.limit_interval_.second       = new_measurment_container->GetFrameCount() - 1LL;

      command_.limit_interval_times_.first  = new_measurment_container->GetTimestamp(command_.limit_interval_.first);
      command_.limit_interval_times_.second = new_measurment_container->GetTimestamp(command_.limit_interval_.second);
    }
    else
    {
      command_.current_frame_timestamp_     = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));
      command_.next_frame_timestamp_        = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));
      command_.first_measurement_timestamp_ = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));
      command_.last_measurement_timestamp_  = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));

      command_.limit_interval_.first        = 0;
      command_.limit_interval_.second       = 0;

      command_.limit_interval_times_.first  = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));
      command_.limit_interval_times_.second = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));
    }
    
    command_.sim_time_                 = command_.first_measurement_timestamp_;
    command_.sim_time_local_timestamp_ = std::chrono::steady_clock::now();

    // Reset the time log
    time_log_.clear();
    time_log_complete_time_span_ = std::chrono::nanoseconds(0);
  }

  {
    // Actually set the measurement
    std::unique_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
    measurement_container_ = std::move(new_measurment_container);
  }

  // Publish the new information as fast as possible
  state_publisher_thread_->PublishNow();
}

bool PlayThread::IsMeasurementLoaded()
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  return (bool)(measurement_container_);
}

std::string PlayThread::GetMeasurementPath()
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  if (measurement_container_)
  {
    return measurement_container_->GetPath();
  }
  else
  {
    return "";
  }
}

std::chrono::nanoseconds PlayThread::GetMeasurementLength()
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  if (measurement_container_)
  {
    return measurement_container_->GetMeasurementLength();
  }
  else
  {
    return std::chrono::nanoseconds(0);
  }
}

std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> PlayThread::GetMeasurementBoundaries()
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  if (measurement_container_ && (measurement_container_->GetFrameCount() > 0))
  {
    return std::make_pair(measurement_container_->GetTimestamp(0), measurement_container_->GetTimestamp(measurement_container_->GetFrameCount() - 1));
  }
  else
  {
    return std::make_pair(eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0)), eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0)));
  }
}

long long PlayThread::GetFrameCount()
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  if (measurement_container_)
  {
    return measurement_container_->GetFrameCount();
  }
  else
  {
    return 0;
  }
}

eCAL::Time::ecal_clock::time_point PlayThread::GetTimestampOf(long long frame_index)
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  if (measurement_container_ && (frame_index >= 0) && (frame_index < measurement_container_->GetFrameCount()))
  {
    return measurement_container_->GetTimestamp(frame_index);
  }
  else
  {
    return eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(-1));
  }
}

std::set<std::string> PlayThread::GetChannelNames()
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  if (measurement_container_)
  {
   return  measurement_container_->GetChannelNames();
  }
  else
  {
    return std::set<std::string>();
  }
}

std::map<std::string, ContinuityReport> PlayThread::CreateContinuityReport()
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  if (measurement_container_)
  {
    return  measurement_container_->CreateContinuityReport();
  }
  else
  {
    return std::map<std::string, ContinuityReport>();
  }
}

std::map<std::string, long long> PlayThread::GetMessageCounters()
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);

  if (measurement_container_)
  {
    return measurement_container_->GetMessageCounters();
  }
  else
  {
    return std::map<std::string, long long>();
  }
}

bool PlayThread::SetLimitInterval(const std::pair<long long, long long>& limit_interval)
{
  return SetLimitInterval_Private(&limit_interval, nullptr);
}

bool PlayThread::SetLimitInterval(const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& limit_interval)
{
  return SetLimitInterval_Private(nullptr, &limit_interval);
}

std::pair<long long, long long> PlayThread::GetLimitInterval()
{
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  return command_.limit_interval_;
}

//////////////////////////////////////////////////////////////////////////////
//// Settings                                                             ////
//////////////////////////////////////////////////////////////////////////////
void PlayThread::SetRepeatEnabled(bool enabled)
{
  EcalPlayLogger::Instance()->info("Setting repeat to:                 " + std::string(enabled ? "True" : "False"));
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  command_.repeat_enabled_ = enabled;
}

void PlayThread::SetPlaySpeed(double speed)
{
  EcalPlayLogger::Instance()->info("Setting play speed to:             " + std::to_string(speed));
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  SetPlayRate_Private(speed);

  pause_cv_.notify_all();
}

void PlayThread::SetLimitPlaySpeedEnabled(bool enabled)
{
  EcalPlayLogger::Instance()->info("Setting limit play speed to:       " + std::string(enabled ? "True" : "False"));
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  command_.limit_play_speed_ = enabled;

  pause_cv_.notify_all();
}

void PlayThread::SetFrameDroppingAllowed(bool allowed)
{
  EcalPlayLogger::Instance()->info("Setting frame dropping to:         " + std::string(allowed ? "True" : "False"));
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  command_.framedropping_allowed_ = allowed;
}

void PlayThread::SetEnforceDelayAccuracyEnabled(bool enabled)
{
  EcalPlayLogger::Instance()->info("Setting enforce delay accuracy to: " + std::string(enabled ? "True" : "False"));
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  command_.enforce_delay_accuracy_ = enabled;
}

bool PlayThread::IsRepeatEnabled()
{
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  return command_.repeat_enabled_;
}

double PlayThread::GetPlaySpeed()
{
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  return command_.play_speed_;
}

bool PlayThread::IsLimitPlaySpeedEnabled()
{
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  return command_.limit_play_speed_;
}

bool PlayThread::IsFrameDroppingAllowed()
{
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  return command_.framedropping_allowed_;
}

bool PlayThread::IsEnforceDelayAccuracyEnabled()
{
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  return command_.enforce_delay_accuracy_;
}


////////////////////////////////////////////////////////////////////////////////
//// Playback                                                               ////
////////////////////////////////////////////////////////////////////////////////

bool PlayThread::Play(long long until_index)
{
  {
    std::unique_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
    if (!measurement_container_)
    {
      return false;
    }

    if (!measurement_container_->PublishersCreated())
    {
      measurement_container_->CreatePublishers();
    }
  }

  {
    std::lock_guard<std::mutex> command_lock(command_mutex_);
    command_.play_until_index_ = until_index;

    SetPlaying_Private(true);
    
    pause_cv_.notify_all();
  }

  return true;
}

bool PlayThread::PlayToNextOccurenceOfChannel(const std::string& source_channel_name)
{
  long long current_index;
  bool repeat_enabled;
  std::pair<long long, long long> limit_interval;
  {
    std::lock_guard<std::mutex> command_lock(command_mutex_);
    current_index  = command_.current_frame_index_;
    repeat_enabled = command_.repeat_enabled_;
    limit_interval = command_.limit_interval_;
  }

  long long until_index = -1;
  {
    std::unique_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
    if (!measurement_container_)
    {
      return false;
    }

    if (!measurement_container_->PublishersCreated())
    {
      measurement_container_->CreatePublishers();
    }

    until_index = measurement_container_->GetNextOccurenceOfChannel(current_index, source_channel_name, repeat_enabled, limit_interval);
  }

  EcalPlayLogger::Instance()->info("Playing to next occurence of channel \"" + source_channel_name + "\"");

  {
    std::lock_guard<std::mutex> command_lock(command_mutex_);
    command_.play_until_index_ = until_index;

    SetPlaying_Private(true);

    pause_cv_.notify_all();
  }

  return true;
}

bool PlayThread::Pause()
{
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  SetPlaying_Private(false);

  pause_cv_.notify_all();
  return true;
}

bool PlayThread::StepForward()
{
  EcalPlayCommand command;

  {
    std::lock_guard<std::mutex> command_lock(command_mutex_);

    // Pause the playback
    SetPlaying_Private(false);

    // Thread-safe copy the command object
    command = command_;
  }

  {
    std::unique_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
    if (!measurement_container_)
    {
      return false;
    }

    if (!measurement_container_->PublishersCreated())
    {
      measurement_container_->CreatePublishers();
    }

    // publish the frame
    measurement_container_->PublishFrame(command.next_frame_index_);

    // Calculate the next frame (We always loop when stepping forward, as it would otherwise not be defined what to do when we reached the end)
    long long next_frame_index = measurement_container_->GetNextEnabledFrameIndex(command.next_frame_index_, true, command.limit_interval_);

    command.current_frame_index_         = command.next_frame_index_;
    command.current_frame_timestamp_     = command.next_frame_timestamp_;
    command.next_frame_index_            = next_frame_index;
    command.next_frame_timestamp_        = measurement_container_->GetTimestamp(command.next_frame_index_);
  }

  {
    std::unique_lock<std::mutex> command_lock(command_mutex_);

    // Step the simtime
    SetSimTime_Private(command_.next_frame_timestamp_);

    // We unlocked the command mutex while publishing the frame and computing
    // the next one. We now copy the data back into the global command object.
    command_.playing_                    = false;
    command_.current_frame_index_        = command.current_frame_index_;
    command_.current_frame_timestamp_    = command.current_frame_timestamp_;
    command_.next_frame_index_           = command.next_frame_index_;
    command_.next_frame_timestamp_       = command.next_frame_timestamp_;
    command_.next_frame_set_manually_    = true;

    // Reset the time log
    time_log_.clear();
    time_log_complete_time_span_ = std::chrono::nanoseconds(0);
  }

  // Publish the new information as fast as possible
  state_publisher_thread_->PublishNow();

  EcalPlayLogger::Instance()->info("Playback step");

  return true;
}

double PlayThread::GetCurrentPlaySpeed()
{
  std::lock_guard<std::mutex> command_lock(command_mutex_); 
  return GetCurrentPlaySpeed_Private();
}

bool PlayThread::JumpTo(long long index)
{
  eCAL::Time::ecal_clock::time_point next_frame_timestamp;
  std::pair<long long, long long>    limit_interval;

  {
    // Get the limit interval
    std::lock_guard<std::mutex> command_lock(command_mutex_);
    limit_interval = command_.limit_interval_;
  }

  {
    std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
    if (!measurement_container_)
    {
      return false;
    }

    // bound the index to valid values
    index = std::min(index, measurement_container_->GetFrameCount() - 1);
    index = std::max(index, 0LL);

    index = std::max(index, limit_interval.first);
    index = std::min(index, limit_interval.second);

    // Get the timestamp
    next_frame_timestamp = measurement_container_->GetTimestamp(index);
  }

  EcalPlayLogger::Instance()->info("Jumping to frame index " + std::to_string(index));

  {
    std::lock_guard<std::mutex> command_lock(command_mutex_);

    command_.next_frame_index_           = index;
    command_.current_frame_timestamp_    = next_frame_timestamp;
    command_.current_frame_index_        = index;
    command_.next_frame_timestamp_       = next_frame_timestamp;
    command_.next_frame_set_manually_    = true;
    
    SetSimTime_Private(next_frame_timestamp);

    // Reset the time log
    time_log_.clear();
    time_log_complete_time_span_ = std::chrono::nanoseconds(0);

    pause_cv_.notify_all();
  }

  // Publish the new information as fast as possible
  state_publisher_thread_->PublishNow();

  return true;
}

bool PlayThread::JumpTo(eCAL::Time::ecal_clock::time_point timestamp)
{
  long long index;
  eCAL::Time::ecal_clock::time_point next_frame_timestamp;
  std::pair<long long, long long>    limit_interval;

  {
    // Get the limit interval
    std::lock_guard<std::mutex> command_lock(command_mutex_);
    limit_interval = command_.limit_interval_;
  }

  {
    std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
    if (!measurement_container_)
    {
      return false;
    }

    // Get index and timestamp
    index = measurement_container_->GetNearestIndex(timestamp);
    index = std::max(index, limit_interval.first);
    index = std::min(index, limit_interval.second);

    next_frame_timestamp = measurement_container_->GetTimestamp(index);
  }

  EcalPlayLogger::Instance()->info("Jumping to timestamp " + std::to_string(std::chrono::duration_cast<std::chrono::duration<double>>(timestamp.time_since_epoch()).count()) + " s");

  {
    std::lock_guard<std::mutex> command_lock(command_mutex_);

    command_.next_frame_index_           = index;
    command_.current_frame_timestamp_    = next_frame_timestamp;
    command_.current_frame_index_        = index;
    command_.next_frame_timestamp_       = next_frame_timestamp;
    command_.next_frame_set_manually_    = true;

    SetSimTime_Private(next_frame_timestamp);

    // Reset the time log
    time_log_.clear();
    time_log_complete_time_span_ = std::chrono::nanoseconds(0);

    pause_cv_.notify_all();
  }

  // Publish the new information as fast as possible
  state_publisher_thread_->PublishNow();
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////
//// State                                                                  ////
////////////////////////////////////////////////////////////////////////////////

EcalPlayState PlayThread::GetCurrentPlayState()
{
  EcalPlayState state;

  std::lock_guard<std::mutex> command_lock_(command_mutex_);
  state.playing_                      = command_.playing_;
  state.actual_play_rate_             = GetCurrentPlaySpeed_Private();
  state.current_frame_index           = command_.current_frame_index_;
  state.current_frame_timestamp       = command_.current_frame_timestamp_;
  if (!command_.limit_play_speed_)
  {
    state.simtime_rate_ = GetCurrentPlaySpeed_Private();
  }
  else
  {
    state.simtime_rate_ = command_.play_speed_;
  }
  state.last_simtime_                 = command_.sim_time_;
  state.last_simtime_local_timestamp_ = command_.sim_time_local_timestamp_;
  return state;
}

bool PlayThread::IsPlaying()
{
  std::lock_guard<std::mutex> command_lock_(command_mutex_);
  return command_.playing_;
}

bool PlayThread::IsPaused()
{
  return !IsPlaying();
}

eCAL::Time::ecal_clock::time_point PlayThread::GetCurrentFrameTimestamp()
{
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  return command_.current_frame_timestamp_;
}

long long PlayThread::GetCurrentFrameIndex()
{
  std::lock_guard<std::mutex> command_lock(command_mutex_);
  return command_.current_frame_index_;
}


////////////////////////////////////////////////////////////////////////////////
//// Publishers                                                             ////
////////////////////////////////////////////////////////////////////////////////

bool PlayThread::InitializePublishers()
{
  return InitializePublishers_Private(false);
}

bool PlayThread::InitializePublishers(const std::map<std::string, std::string>& channel_mapping)
{
  return InitializePublishers_Private(true, channel_mapping);
}

bool PlayThread::DeInitializePublishers()
{
  {
    std::lock_guard<std::mutex> command_lock(command_mutex_);
    SetPlaying_Private(false);
  }

  std::unique_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  if (measurement_container_)
  {
    EcalPlayLogger::Instance()->info("De-Initializing eCAL publishers");
    measurement_container_->DeInitializePublishers();
    return true;
  }
  return false;
}

bool PlayThread::PubishersInitialized()
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  return measurement_container_ && measurement_container_->PublishersCreated();
}

std::map<std::string, std::string> PlayThread::GetChannelMapping()
{
  std::shared_lock<std::shared_timed_mutex> measurement_lock(measurement_mutex_);
  if (measurement_container_)
  {
    return measurement_container_->GetChannelMapping();
  }
  else
  {
    return std::map<std::string, std::string>();
  }
}
