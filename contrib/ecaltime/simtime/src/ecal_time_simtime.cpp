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

/**
 * @brief  eCALTime simulation time interface
**/

#include "ecal_time_simtime.h"

eCAL::CSimTime::CSimTime() :
  is_initialized(false),
  is_synchronized(false),
  first_message_received(false),
  play_state(eCAL::pb::SimTime_eState::SimTime_eState_stopped),
  last_measurement_time(0),
  time_of_last_measurement_time(0),
  play_speed(0.0)
{
  sleeper.setRate(play_speed);
}

bool eCAL::CSimTime::initialize()
{
  std::unique_lock<std::mutex> lk(initialize_mutex);
  if (!is_initialized)
  {
    //eCAL::Initialize("ecal_sim_time_listener", eCAL::Init::Subscriber);
    // this has to be done by the parent process
    // needs to be fixed with an improved reference counting
    // in eCAL::Initialize ..

    sim_time_subscriber = std::make_unique<eCAL::protobuf::CSubscriber<eCAL::pb::SimTime>>("__sim_time__");
    sim_time_subscriber->SetReceiveCallback(std::bind(&eCAL::CSimTime::onSimTimeMessage, this, std::placeholders::_2));
    is_initialized = true;
    return true;
  }
  else
  {
    return false;
  }
}

bool eCAL::CSimTime::finalize()
{
  // the time interface is part of the finalization
  // process and we get circular calls ...
  // needs to be fixed with an improved reference counting
  // in eCAL::Finalize ..
#if 0
  if (eCAL::Finalize() == 0) {
    return true;
  }
  else {
    return false;
  }
#else
  return true;
#endif
}

void eCAL::CSimTime::sleep_for(long long duration_nsecs_)
{
  sleeper.sleepFor(duration_nsecs_);
}

bool eCAL::CSimTime::isSynchronized()
{
  return is_synchronized;
}

bool eCAL::CSimTime::getMasterTime(uint64_t & time_)
{
  std::unique_lock<std::mutex> lk(time_mutex);

  double elapsed_sim_time = (double)((long long)getCurrentNanos() - time_of_last_measurement_time) * play_speed;
  time_ = (uint64_t)((unsigned long long) last_measurement_time + (unsigned long long)elapsed_sim_time);

  return true;
}

void eCAL::CSimTime::getStatus(int & error_, std::string * status_message_)
{
  {
    std::unique_lock<std::mutex> lk(initialize_mutex);
    if (!is_initialized) {
      error_ = SimtimeError::NOT_INITIALIZED;
      if (status_message_) {
        status_message_->assign("SimTime has not been initialized");
      }
      return;
    }
  }
  {
    std::unique_lock<std::mutex> lk(time_mutex);
    if (!first_message_received) {
      error_ = SimtimeError::NO_MESSAGES_RECEIVED;
      if (status_message_) {
        status_message_->assign("SimTime hasn't recieved any messages, yet");
      }
    }
    else {
      error_ = 0;
      if (status_message_) {
        status_message_->assign("SimTime is OK. Current status is ");
        switch (play_state)
        {
        case eCAL::pb::SimTime_eState::SimTime_eState_paused:
          status_message_->append("Paused");
          break;
        case eCAL::pb::SimTime_eState::SimTime_eState_playing:
          status_message_->append("Playing");
          break;
        case eCAL::pb::SimTime_eState::SimTime_eState_stopped:
          status_message_->append("Stopped");
          break;
        default:
          status_message_->append("Unknown");
          break;
        }
      }
    }
  }
}

void eCAL::CSimTime::onSimTimeMessage(const eCAL::pb::SimTime & sim_time_)
{
  first_message_received = true;
  long long receive_time = (long long) getCurrentNanos();
  {
    std::unique_lock<std::mutex> lk(time_mutex);
    last_measurement_time = sim_time_.simulation_time_nsecs();
    if (sim_time_.host_name() == eCAL::Process::GetHostName()) {
      time_of_last_measurement_time = sim_time_.local_time_nsecs();
    }
    else {
      time_of_last_measurement_time = receive_time;
    }
    play_speed = sim_time_.real_time_factor();
    play_state = sim_time_.simulation_state();
    if (sim_time_.simulation_state() == eCAL::pb::SimTime::eState::SimTime_eState_stopped) {
      is_synchronized = false;
      play_speed = 0;
    }
    else if (sim_time_.simulation_state() == eCAL::pb::SimTime::eState::SimTime_eState_paused) {
      is_synchronized = true;
      play_speed = 0;
    }
    else {
      is_synchronized = true;
    }
    sleeper.setTimeAndRate(last_measurement_time, play_speed);
  }
}


unsigned long long eCAL::CSimTime::getCurrentNanos() {
  std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
  unsigned long long nanos = currentTime.time_since_epoch().count();
  return nanos;
}
