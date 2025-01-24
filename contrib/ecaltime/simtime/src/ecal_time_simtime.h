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

/**
 * @brief  eCALTime simulation time interface
**/

#pragma once

#include <stdint.h>
#include <mutex>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/subscriber.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/ecaltime/pb/sim_time.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "dynamic_sleeper.h"

namespace eCAL
{
  /**
   * @brief  Simulation time interface that listens on the __sim_time__ topic
  **/
  class CSimTime
  {
  public:
    /**
     * @brief Constructor for the time interface
     */
    CSimTime();

    /**
     * @brief initializes the time interface by creating the eCAL Subscriber
     * @return true if initializing was successful and false if initializing
     * was not successful or the time interface had already been initialized
     */
    bool initialize();

    /**
     * @brief Checks if the time interface is synchronized, i.e. whether any node
     * publishes timestamps and the time is running
     *
     * @return true if the time interface is synchronized
     */
    bool isSynchronized();

    /**
     * @brief Get the current simulation time
     * @param[out] time_ the current simulation time
     * @return true if getting the time was successful and time_ containing a valid time.
     */
    bool getMasterTime(uint64_t& time_);

    /**
     * @brief Gets the current status of this adapter
     * @param[out] error_          The error code (0 if everything is fine)
     * @param[out] status_message_ The current status as human readable string
     */
    void getStatus(int& error_, std::string* status_message_);

    /**
     * @brief Finalizes the time interface
     * @return true if finalizing the time interface was successful
     */
    bool finalize();

    /**
    * @brief Blocks for the given amount of nanoseconds.
    *
    * The time base for the duration is the simulation time that may proceed at
    * an arbitrary rate. Therefore, the actual time will vary.
    */
    void sleep_for(long long duration_nsecs_);


  private:

    enum SimtimeError
    {
      NO_ERROR = 0,
      NOT_INITIALIZED = 1,
      NO_MESSAGES_RECEIVED = 2,
    };

    bool is_initialized;                        /**< Whether the initialize function has already been called */
    bool is_synchronized;                       /**< Whether the time interface is synchronized, i.e. whether any node publishes timestamps and the time is running */
    std::mutex initialize_mutex;                /**< Mutex for initialization */

    bool first_message_received;                /**< Whether we received at least one Message (used for the status message)*/
    eCAL::pb::SimTime::eState play_state;         /**< Current state (used for the status message)*/

    std::unique_ptr<eCAL::protobuf::CSubscriber<eCAL::pb::SimTime>> sim_time_subscriber; /**< Subscriber for getting simulation timestamps */

    std::mutex time_mutex;                      /**< Mutex for computing the current simulation time */
    long long last_measurement_time;            /**< Last received simulation time */
    long long time_of_last_measurement_time;    /**< The time when the the last simulation timestamp was received */
    double play_speed;                          /**< Realtime factor at which the time has to proceed */

    CDynamicSleeper sleeper;                    /**< The dynamic sleeper that handles changes of the play speed while processes are sleeping */

    /**
     * @brief Callback for the eCAL Subscriber
     * @param sim_time_ a simtime message
     */
    void onSimTimeMessage(const class eCAL::pb::SimTime& sim_time_);

    /**
     * @brief Calculates the current time in nanoseconds since an abstract epoch
     * @return the current time in nanoseconds
     */
    unsigned long long getCurrentNanos();
  };
}
