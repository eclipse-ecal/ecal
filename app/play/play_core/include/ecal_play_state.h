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

#pragma once

#include <chrono>
#include <cfloat>
#include <math.h>

#include <ecal/ecal.h>

/**
 * @brief A container carrying information about eCAL Play's current state
 *
 * A state contains information whether the player is currently playing a
 * measurement, the actual rate at which it is playing, the last published frame
 * and the simulation time.
 * Note, that in order to percieve a "current" simulation time, the simtime
 * values have to be interpolated.
 */
struct EcalPlayState
{
  bool                                  playing_ = false;                       /**< Whether the player is currently playing a measurement */
  double                                actual_play_rate_ = 0.0;                /**< The current actual play rate. This may differ from the selected play rate. The play rate is relative, i.e. a play rate of 1.0 represents realtime. */
  
  long long                             current_frame_index = 0;                /**< The index of the last frame that was published. In cases where the player has just been started or the measurement position has been changed manually, this frame may not have been published, yet. In these cases, it represents the next frame. */
  eCAL::Time::ecal_clock::time_point    current_frame_timestamp;                /**< The timestamp of the last frame that was published. In cases where the player has just been started or the measurement position has been changed manually, this frame may not have been published, yet. In these cases, it represents the next frame. */

  eCAL::Time::ecal_clock::time_point    last_simtime_;                          /**< The last sim time. This value will not be updated continuously. In order to compute the current sim time, interpolate this time using the @see{last_simtime_local_timestamp_} and the @see{simtime_rate_}. */
  std::chrono::steady_clock::time_point last_simtime_local_timestamp_;          /**< The local timestamp of the simtime @see{last_simtime_}. */
  double                                simtime_rate_ = 0.0;                    /**< The rate at which the simetime proceeds / has proceeded since the last @see{last_simtime_local_timestamp_}. A rate of 1.0 represents realtime.*/

  bool operator==(const EcalPlayState& other) const
  {
    return (playing_                    == other.playing_)
      && (fabs(actual_play_rate_        -  other.actual_play_rate_) < DBL_EPSILON)
      && (current_frame_index           == other.current_frame_index)
      && (current_frame_timestamp       == other.current_frame_timestamp)
      && (last_simtime_                 == other.last_simtime_)
      && (last_simtime_local_timestamp_ == other.last_simtime_local_timestamp_)
      && (fabs(simtime_rate_            -  other.simtime_rate_) < DBL_EPSILON);
  }
  bool operator !=(const EcalPlayState& other) const
  {
    return !operator==(other);
  }
};
