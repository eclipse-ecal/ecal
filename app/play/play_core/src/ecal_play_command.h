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
#include <deque>
#include <ecal/ecal.h>

struct EcalPlayCommand
{
  EcalPlayCommand()
    : playing_                    (false)
    , play_speed_                 (1.0)
    , limit_play_speed_           (true)
    , repeat_enabled_             (false)
    , framedropping_allowed_      (false)
    , play_until_index_           (-1)

    , limit_interval_             (0LL, 0LL)
    , limit_interval_times_       (eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0)), eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0)))

    , next_frame_set_manually_    (false)
    , next_frame_index_           (-1)
    , next_frame_timestamp_       (std::chrono::nanoseconds(0))

    , current_frame_index_        (-1)
    , current_frame_timestamp_    (std::chrono::nanoseconds(0))

    , last_measurement_timestamp_ (std::chrono::nanoseconds(0))
    , first_measurement_timestamp_(std::chrono::nanoseconds(0))

    , enforce_delay_accuracy_     (false)
    , sim_time_local_timestamp_   (std::chrono::nanoseconds(0))
    , sim_time_                   (std::chrono::nanoseconds(0))
  {}

  bool      playing_;                                                           /**< Whether the playback is currently running */
  double    play_speed_;                                                        /**< The current (target) play speed. Not respected, if the limit_play_speed_ is set to false */
  bool      limit_play_speed_;                                                  /**< Whether the play speed should be limited to the play_speed_ or all frames should be published as fast as possible */
  bool      repeat_enabled_;                                                    /**< Restart the from the front, if it has reached the end */
  bool      framedropping_allowed_;                                             /**< Allow the player to drop frames in order to keep the temporal accuracy of the remaining frames*/
  long long play_until_index_;                                                  /**< The playback shall stop when reaching this index */

  std::pair<long long, long long>       limit_interval_;                        /**< The interval with a start and an end index. The playback will be limited to this range */
  std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> limit_interval_times_; /**< The timestamps of the frames stored in the limit_interval_. Used for not having to lock the measurement mutex */

  bool                                  next_frame_set_manually_;               /**< When setting the position during a playback, this variable prevents auto-calculation of the next frame at the end of the loop */
  long long                             next_frame_index_;                      /**< Which frame to publish next */
  eCAL::Time::ecal_clock::time_point    next_frame_timestamp_;                  /**< Used to calculate how long to wait for the next frame */

  long long                             current_frame_index_;                   /**< The last frame that was published (or the next frame that will be published in cases when the player as just jumped to a new position). Used to determine wether the player is wrapping around the end when repeat is enabled. */
  eCAL::Time::ecal_clock::time_point    current_frame_timestamp_;               /**< The timestamp of the current_frame_index_ */
     
  eCAL::Time::ecal_clock::time_point    last_measurement_timestamp_;            /**< When repeating, we need to sleep from now->last->next frame and set the time to the first frame in between. Although this information can also be determined directly from the measurement, we store it here for not having to lock the measurement mutex. */
  eCAL::Time::ecal_clock::time_point    first_measurement_timestamp_;           /**< When repeating, we need to sleep from now->last->next frame and set the time to the first frame in between. Although this information can also be determined directly from the measurement, we store it here for not having to lock the measurement mutex. */

  bool                                  enforce_delay_accuracy_;                /**< Force the player to always respect the time between two frames, even if the last frame has been delayed. */
  std::chrono::steady_clock::time_point sim_time_local_timestamp_;              /**< The local timestamp of the sim_time_ timestamp. Used for interpolating the current sim time with the play_speed_ */
  eCAL::Time::ecal_clock::time_point    sim_time_;                              /**< A simulation timestamp that was valid at the sim_time_local_timestamp_ timestamp. Has to be interpolated with theplay_speed_ to get the current simulation time */
};
