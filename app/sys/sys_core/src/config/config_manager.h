/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "ecalsys/ecal_sys.h"
#include "ecalsys/config/config_version.h"

class ConfigManager
{
public:
  /** This class only provides static functions and is not meant to be instanciated */
  ConfigManager() = delete;
  virtual ~ConfigManager();

  /**
   * @brief Loads an eCAL Sys configuration file and populates the given ecalsys instance with that data.
   *
   * Loads an eCAl Sys configuration file.
   * If the file does exist and can be loaded, the ecalsys instance will be
   * cleared and populated with the data from the configuraiton file (At least
   * for the default case, where append=false).
   * If the file cannot be loaded, an exception is thrown and the ecalsys
   * instance will remain in its previous state. The exception contains a human
   * readable message.
   *
   * Currently, only eCAl Sys 1.0 Configs are supported, as this is the only
   * existing format, right now. As eCAL Sys 2.0 however uses a different
   * internal architecture, some adjustments will be made:
   *
   *    - Runners with the name "exe" or "bat" that have an empty config will
   *      be stripped, as those runners did not carry any information anyway.
   *
   *    - eCAL Sys 1.0 functions are interpreted as eCAL Sys 2.0 Groups
   *
   *    - The restart-below-severity will be shifted by one level and
   *      converted to a restart-at-severity, which will cause clipping at
   *      Failed Lv.5
   *
   *    - Window size and position is not supported
   *
   * @param ecalsys The ecalsys instance that shall be populated with the data from the config file
   * @param path    The path to the configuration file to load
   * @param append  If true, the config will be appended to the existing on. IDs may have to be re-mapped, if they already exist. Options will not be set.
   *
   * @throws std::runtime_error   If loading the configuration file has failed
   *
   * @return True, if loading the config has been successfull
   */
  static bool LoadConfig(EcalSys& ecalsys, const std::string& path, bool append = false);

  /**
   * @brief Saves the configuration from the given ecalsys instance to a configuration file
   *
   * Saves the configuration to a file. If the file cannot be saved, an
   * exception is thrown, which contains a human readable exception message.
   *
   * Currently, only eCAl Sys 1.0 Configs are supported, as this is the only
   * existing format, right now. As eCAL Sys 2.0 however uses a different
   * internal architecture, some adjustments will be made:
   *
   *    - All tasks without runner will get a dummy "exe" runner assigned, that
   *      does not carry any information, but is necessary to achieve backwards
   *      compatibility with eCAL Sys 1.0
   *
   *    - Task groups are saved as eCAL Sys 1.0 functions
   *
   *    - The restart-at-severity of minimal group states will be shifted by one
   *      level and converted to a restart-below-severity, which will cause
   *      clipping at Unknown Lv.1
   *
   *    - The position of minimal group states in their according list will be
   *      converted to the eCAL Sys 1.0 function state priority
   *
   *    - If the original loaded config did contain any imported files, the
   *      whole content of that config will be saved to the same file along the
   *      rest of the config. Imported files are an unsupported legacy feature
   *      at the moment.
   *
   *    - The fields for the window size and position will be left empty
   *
   * @param ecalsys   The EcalSys instance to save to ta file
   * @param path      The path where to save the config to
   * @param version   The target config version. Currently, only v1_0 is supported.
   *
   * @throws std::runtime_error   If saving the configuration file has failed
   *
   * @return True, if saving the config has been successfull.
   */
  static bool SaveConfig(EcalSys& ecalsys, const std::string& path, ConfigVersion version = ConfigVersion::v1_0);
};
