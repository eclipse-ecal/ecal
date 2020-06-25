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

#include <SimpleIni.h>

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <ecal/ecal_util.h>

namespace LinuxPtpConfig {

  /**
   * @brief reads the file ~/.ecal/ecaltime.ini to get the device
   * @return the device value from the linuxptp section
   */
  std::string getDevice() {
    CSimpleIniA ini;

    std::string path_to_ini = eCAL::Util::GeteCALConfigPath();
    path_to_ini += "ecaltime.ini";

    int err = ini.LoadFile(path_to_ini.c_str());
    if (err != SI_OK){
      std::cerr << "Error reading ecaltime config file" << std::endl;
    }

    const char * pVal = ini.GetValue("linuxptp", "device", "/dev/ptp0");

    std::string device(pVal);
    return device;
  }
}
