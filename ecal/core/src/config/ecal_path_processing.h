/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  Function definitions for path processing
**/

#include <string>

namespace eCAL
{
  namespace Path
  {
    /**
   * @brief Check the following paths if the specified config file exists and return the first valid complete path
   * 
   *          The function checks the following paths in order:
   *          1. Current working directory
   *          2. ECAL_DATA environment variable path
   *          3. CMake configured data paths (Linux only)
   *          4. System data path
   * 
   * @param config_file_ The name of the configuration file to check.
   * @return std::string The first valid complete path to the configuration file.
   */
    std::string checkForValidConfigFilePath(const std::string& config_file_);
  }
}