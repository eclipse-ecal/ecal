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

#include "state_publisher_thread.h"

#include <ecal/ecal.h>

#include "play_thread.h"

#include <iostream>

StatePublisherThread::StatePublisherThread(PlayThread& play_thread)
  : InterruptibleThread()
  , play_thread_(play_thread)
  , state_publisher_("__ecalplay_state__")
  , simtime_publisher_("__sim_time__")
{}

StatePublisherThread::~StatePublisherThread()
{}

void StatePublisherThread::Interrupt()
{
  InterruptibleThread::Interrupt();
  loop_cv_.notify_all();
}

void StatePublisherThread::PublishNow()
{
  loop_cv_.notify_all();
}

void StatePublisherThread::StatePublisherThread::Run()
{
  std::unique_lock<std::mutex> loop_lock(loop_mutex_);

  while (!IsInterrupted())
  {
    loop_cv_.wait_for(loop_lock, std::chrono::milliseconds(500));
    if (IsInterrupted()) return;
    PublishState();
    if (IsInterrupted()) return;
    PublishSimtime();
  }
}

void StatePublisherThread::PublishState()
{
  EcalPlayState state = play_thread_.GetCurrentPlayState();
  
  eCAL::pb::play::State state_pb;

  state_pb.set_host_name(eCAL::Process::GetHostName());
  state_pb.set_process_id(eCAL::Process::GetProcessID());
  state_pb.set_playing(state.playing_);
  state_pb.set_measurement_loaded(play_thread_.IsMeasurementLoaded());
  state_pb.set_actual_speed(state.actual_play_rate_);
  state_pb.set_current_measurement_index(state.current_frame_index);
  state_pb.set_current_measurement_timestamp_nsecs(std::chrono::duration_cast<std::chrono::nanoseconds>(state.current_frame_timestamp.time_since_epoch()).count());

  if (state_pb.measurement_loaded())
  {
    auto meas_info = state_pb.mutable_measurement_info();
    meas_info->set_path                 (play_thread_.GetMeasurementPath());
    meas_info->set_frame_count          (play_thread_.GetFrameCount());

    auto meas_boundaries = play_thread_.GetMeasurementBoundaries();
    meas_info->set_first_timestamp_nsecs(std::chrono::duration_cast<std::chrono::nanoseconds>(meas_boundaries.first.time_since_epoch()).count());
    meas_info->set_last_timestamp_nsecs (std::chrono::duration_cast<std::chrono::nanoseconds>(meas_boundaries.second.time_since_epoch()).count());
  }

  auto settings = state_pb.mutable_settings();
  settings->set_play_speed                    (play_thread_.GetPlaySpeed());
  settings->set_limit_play_speed              (play_thread_.IsLimitPlaySpeedEnabled());
  settings->set_repeat_enabled                (play_thread_.IsRepeatEnabled());
  settings->set_framedropping_allowed         (play_thread_.IsFrameDroppingAllowed());
  settings->set_enforce_delay_accuracy_enabled(play_thread_.IsEnforceDelayAccuracyEnabled());
  auto limit_interval = play_thread_.GetLimitInterval();
  settings->set_limit_interval_lower_index    (limit_interval.first);
  settings->set_limit_interval_upper_index    (limit_interval.second);

  state_publisher_.Send(state_pb);
}

void StatePublisherThread::PublishSimtime()
{
  EcalPlayState state = play_thread_.GetCurrentPlayState();

  eCAL::pb::SimTime simtime_pb;

  if (play_thread_.IsMeasurementLoaded())
  {
    if (state.playing_)
    {
      simtime_pb.set_simulation_state(eCAL::pb::SimTime::eState::SimTime_eState_playing);
    }
    else
    {
      simtime_pb.set_simulation_state(eCAL::pb::SimTime::eState::SimTime_eState_paused);
    }
  }
  else
  {
    simtime_pb.set_simulation_state(eCAL::pb::SimTime::eState::SimTime_eState_stopped);
  }

  simtime_pb.set_simulation_time_nsecs(std::chrono::duration_cast<std::chrono::nanoseconds>(state.last_simtime_.time_since_epoch()).count());
  simtime_pb.set_real_time_factor     (state.simtime_rate_);
  simtime_pb.set_local_time_nsecs     (std::chrono::duration_cast<std::chrono::nanoseconds>(state.last_simtime_local_timestamp_.time_since_epoch()).count());
  simtime_pb.set_host_name            (eCAL::Process::GetHostName());
  simtime_pb.set_process_id           (eCAL::Process::GetProcessID());

  simtime_publisher_.Send(simtime_pb);
}