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

#include <string>
#include <mutex>

namespace Udpcap
{
  static std::mutex pcap_compile_mutex; // pcap_compile is not thread safe, so we need a global mutex

  /**
   * @brief Initializes Npcap, if not done already. Must be called before calling any native npcap methods.
   *
   * This method initialized Npcap and must be called at least once before
   * calling any ncap functions.
   * As it always returns true when npcap has been intialized successfully, it
   * can also be used to check whether npcap is available and working properly.
   *
   * If this function returns true, npcap should work.
   *
   * @return True if npcap is working
   */
  bool Initialize();
    
  /**
   * @brief Checks whether npcap has been initialized successfully
   * @return true if npcap has been initialized successfully
   */
  bool IsInitialized();

  /**
   * @brief Gets the device name of the npcap loopback device as read from the registry
   *
   * The device name has the form: \device\npcap_{6DBF8591-55F9-4DEF-A317-54B9563A42E3}
   * If a modern NPCAP version has been installed without legacy loopback support,
   * The device name will always be \device\npf_loopback
   *
   * @return The name of the loopback device
   */
  std::string GetLoopbackDeviceName();

  /**
   * @brief Gets the UUID of the npcap loopback device as read from the registry
   *
   * The UUID has the form 6DBF8591-55F9-4DEF-A317-54B9563A42E3
   *
   * @return The UUID of the loopback device (or "", if the device could not be determined)
   */
  std::string GetLoopbackDeviceUuidString();

  /**
   * @brief Checks for a given device name, if it is the npcap loopback device
   *
   * @param device_name  The entire device name
   *
   * @return True, if the device matches the NPCAP loopback device.
   */
  bool IsLoopbackDevice(const std::string& device_name);

  /**
   * @brief Returns a human readible status message.
   *
   * This message is intended to be displayed in a graphical user interface.
   * For terminal based applications it is not needed, as the messages are also
   * printed to stderr.
   *
   * @return The Udpcap status as human-readible text (may be multi-line)
   */
  std::string GetHumanReadibleErrorText();
}
