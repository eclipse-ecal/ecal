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

#include <time.h>
#include <mutex>

#define CLOCKFD 3
#define FD_TO_CLOCKID(fd)   ((~(clockid_t) (fd) << 3) | CLOCKFD)
#define CLOCKID_TO_FD(clk)  ((unsigned int) ~((clk) >> 3))

class Linuxptp
{
public:
  Linuxptp();

  /**
   * @brief opens the (PTP) hardware clock
   * @param[in]   device  A path to the clock to open (e.g. /dev/ptp0)
   * @return 0 if succeeded or -1 if failed. If the function fails, the error is written as stderr.
   */
  int initialize(std::string device);

  /**
   * @brief closes the (PTP) clock
   * @return 0 on success or -1 if failed. If the function fails, the error is written as stderr
   */
  int finalize();

  /**
   * @brief queries the clock and returns it's duration since the epoch in nanoseconds
   * @return the current time (or 0, if an error occurred)
   */
  long long getCurrentNsecs();

  /**
   * @brief Get the current error code and status message
   *
   * An error code of 0 is considered to be OK. Any other error code is
   * considered to indicate a problem.
   * The Status message may be a nullpointer.
   *
   * @param error_ [out]          the error code
   * @param status_message_ [out] a human-readable status message. May be nullptr.
   */
  void getStatus(int& error_, std::string* status_message_);

private:
  clockid_t clock_id;         /** < A handle to the current clock */
  bool is_initialized;        /** < Wether this time adapter has been successfully initialized */
  std::mutex clock_mutex;     /** < Mutex to prevent different threads from simultaniously initializing or using the clock */

  std::string status_message; /** < The last status message */
  int error_code;             /** < The last error code */
  std::mutex status_mutex;    /** < Mutex for protecting the status message and error code*/

  /**
   * @brief Queries the Capabilities of the given PTP Clock
   * @param[in] clockId The PTP Clock of interest
   * @param[out] caps The capabilities of the PTP Clock
   * @return 0 if succeeded or -1 if failed. If the function fails, errno is set accordingly.
   */
  static int getPtpClockCaps(const clockid_t clockId, struct ptp_clock_caps *caps);

  /**
   * @brief opens the (PTP) hardware clock
   * @param[in]   device  A path to the clock to open (e.g. /dev/ptp0)
   * @param[out]  clockId The ID to access the clock. Only valid if the function succeeded.
   * @return 0 if succeeded or -1 if failed. If the function fails, errno is set accordingly.
   */
  int openClock(const char *device, clockid_t *clockId);
};
