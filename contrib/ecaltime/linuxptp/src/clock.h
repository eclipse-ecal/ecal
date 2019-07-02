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

#include <sys/ioctl.h>
#include <linux/ptp_clock.h>
#include <fcntl.h>
#include <unistd.h>

#define CLOCKFD 3
#define FD_TO_CLOCKID(fd)   ((~(clockid_t) (fd) << 3) | CLOCKFD)
#define CLOCKID_TO_FD(clk)  ((unsigned int) ~((clk) >> 3))

namespace linuxptp{
/**
 * @brief Queries the Capabilities of the given PTP Clock
 * @param[in] clockId The PTP Clock of interest
 * @param[out] caps The capabilities of the PTP Clock
 * @return 0 if succeeded or -1 if failed. If the function fails, errno is set accordingly.
 */
static int getPtpClockCaps(const clockid_t clockId, struct ptp_clock_caps *caps){
    int err = ioctl(CLOCKID_TO_FD(clockId), PTP_CLOCK_GETCAPS, caps);
    return err;
}

/**
 * @brief opens the (PTP) hardware clock
 * @param[in]   device  A path to the clock to open (e.g. /dev/ptp0)
 * @param[out]  clockId The ID to access the clock. Only valid if the function succeeded.
 * @return 0 if succeeded or -1 if failed. If the function fails, errno is set accordingly.
 */
int openClock(const char *device, clockid_t *clockId){

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

/**
 * @brief closes the (PTP) clock
 * @param[in] clockId the clock ID to close
 * @return 0 on success or -1 if failed. If the function fails, errno is set accordingly.
 */
int closeClock(clockid_t clockId){
    return close(CLOCKID_TO_FD(clockId));
}
}
