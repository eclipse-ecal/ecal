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

#include "ThreadingUtils/InterruptibleThread.h"

#include <mutex>
#include <condition_variable>

#include <ecal/msg/protobuf/publisher.h>
#include <ecal/ecal.h>
#include <ecal/pb/play/state.pb.h>
#include <ecal/pb/sim_time.pb.h>

class PlayThread;

/**
 * @brief A Thread that periodically retrieves the curren state from eCAL Play and pubishes the information on the __sim_time__ and __ecalplay_state__ topic
 */
class StatePublisherThread : public InterruptibleThread
{
public:
  /**
   * @brief Creates a new State Publisher Thread that will publish the state every 500ms once Start() has been called
   * @param play_thread    The play thread to retrieve the state information from
   */
  StatePublisherThread(PlayThread& play_thread);

  ~StatePublisherThread();

  void Interrupt() override;

  /**
   * @brief Immediatelly retrieve state information and publish them, without waiting for the next loop
   */
  void PublishNow();

protected:
  void Run() override;

private:
  PlayThread& play_thread_;                                                     /**< The Play thread to get the state information from */

  std::mutex loop_mutex_;                                                       /**< The mutex for the condition variable. Used for waiting until the next loop. */
  std::condition_variable loop_cv_;                                             /**< The condition variable used for waiting until the next loop. Can be interrupted in order to immediatelly publish the information (e.g. in cases when the player is paused) */

  eCAL::protobuf::CPublisher<eCAL::pb::play::State> state_publisher_;                /**< Publishes the __ecalplay_state__ topic */
  eCAL::protobuf::CPublisher<eCAL::pb::SimTime>     simtime_publisher_;              /**< Publishes the __sim_time__ topic */

  /**
   * @brief Publish the __ecalplay_state__ topic
   */
  void PublishState();

  /**
   * @brief Publish the __sim_time__ topic
   */
  void PublishSimtime();
};
