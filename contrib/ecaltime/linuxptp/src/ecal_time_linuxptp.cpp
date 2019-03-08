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

#include "ecal_time_linuxptp.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <linux/ptp_clock.h>
#include <errno.h>
#include <string.h>
#include <iostream>

Linuxptp::Linuxptp():
  is_initialized(false),
  status_message("linuxptp adapter has not been initialized"),
  error_code(-1)
{}

int Linuxptp::initialize(std::string device){
  std::unique_lock<std::mutex> lk (clock_mutex);

  if(is_initialized){
    return 0;
  }

  int err;

  if (device == "CLOCK_REALTIME"){
    clock_id = CLOCK_REALTIME;
    err = 0;
  } else if (device == "CLOCK_MONOTONIC"){
    clock_id = CLOCK_MONOTONIC;
    err = 0;
  } 
#ifdef CLOCK_TAI
    else if (device == "CLOCK_TAI"){
    clock_id = CLOCK_TAI;
    err = 0;
  }
#endif /* CLOCK_TAI */
    else {
    err = openClock(device.c_str(), &clock_id);
  }

  {
    std::unique_lock<std::mutex> status_lk(status_mutex);
    if (err){
      error_code = errno;
    }else{
      error_code = 0;
    }

    if(err){
      status_message.assign("Failed to open clock ");
      status_message += "\"";
      status_message += device;
      status_message += "\": ";
      status_message += strerror(errno);
      std::cerr << status_message << std::endl;

      is_initialized = false;
    } else {
      status_message.assign("Linux PTP Adapter is OK. Using device \"");
      status_message += device + "\".";
      is_initialized = true;
    }
  }
  return err;
}

int Linuxptp::finalize(){
  std::unique_lock<std::mutex> lk (clock_mutex);
  is_initialized = false;
  int err = close(CLOCKID_TO_FD(clock_id));
  if (err){
    perror("Error closing clock");
  }
  return err;
}

long long Linuxptp::getCurrentNsecs(){
  std::unique_lock<std::mutex> lk (clock_mutex);
  if(is_initialized){
    timespec ts;
    int err = clock_gettime(clock_id, &ts);
    long long nanoseconds = (long long)ts.tv_nsec + (long long)ts.tv_sec * 1000000000LL;
    if(err){
      error_code = errno;
      status_message.assign("An error occured getting the current time: ");
      status_message += strerror(errno);
    }
    return nanoseconds;
  } else{
    return 0;
  }
}

int Linuxptp::openClock(const char *device, clockid_t *clockId){

  struct ptp_clock_caps caps;
  int file_descriptor = open(device, O_RDONLY);

  if (file_descriptor == -1){
    return -1;
  }else{
    *clockId = FD_TO_CLOCKID(file_descriptor);
    // Check if the 'thing' we just opened actually is a PTP clock
    if(getPtpClockCaps(*clockId, &caps)){
       close(file_descriptor);
      return -1;
    }else{
      return 0;
    }
  }
}

void Linuxptp::getStatus(int& error_, std::string* status_message_){
  std::unique_lock<std::mutex> status_lk(status_mutex);
  error_ = error_code;
  if(status_message_){
    status_message_->assign(status_message.c_str());
  }
}

int Linuxptp::getPtpClockCaps(const clockid_t clockId, struct ptp_clock_caps *caps){
  int err = ioctl(CLOCKID_TO_FD(clockId), PTP_CLOCK_GETCAPS, caps);
  return err;
}
