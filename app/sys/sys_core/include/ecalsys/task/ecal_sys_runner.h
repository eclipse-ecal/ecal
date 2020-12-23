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
#include <mutex>

/**
 * @brief A container for all information regarding an eCAL Sys Runner
 *
 * A Runner usually is an executable that dynamically loads a .dll / .so file.
 * Thus, a runner mainly consists of a path to that executable and a command
 * line argument for loading that file.
 *
 * Getting and setting values of a runner is thread safe.
 */
class EcalSysRunner
{
public:
  /**
   * @brief Creates a new eCAL Sys Runner with empty values and a randomly generated id.
   */
  EcalSysRunner();

  /**
   * @brief Crates a new eCAL Sys Runner with the given values
   *
   * @param id                  The ID of the runner
   * @param name                The name of the runner (mainly for displaying it in the GUI)
   * @param path                Path to the runner-executable
   * @param default_algo_dir    Default path where the .dll / .so files are located
   * @param load_cmd_argument   Command line argument for loading files
   */
  EcalSysRunner(uint32_t id, const std::string& name, const std::string& path, const std::string& default_algo_dir, const std::string& load_cmd_argument);

  ~EcalSysRunner();

  /**
   * @brief Sets the ID of the runner
   * @param id The new ID of the runner
   */
  void SetId(uint32_t id);

  /**
   * @brief Sets the name of the runner, that is displayed in the GUI
   * @param name The new name of the runner
   */
  void SetName(const std::string& name);

  /**
   * @brief Sets the path to the runner's executable
   * @param path The new executable path
   */
  void SetPath(const std::string& path);

  /**
   * @brief Sets the default directory where the .dll / .so files associdated with this runner are located
   * @param default_algo_dir the new default algorithm directory
   */
  void SetDefaultAlgoDir(const std::string& default_algo_dir);

  /**
   * @brief Sets the command line argument that is used to load files with this runner (e.g. "--dll")
   * @param load_cmd_argument the new command line
   */
  void SetLoadCmdArgument(const std::string& load_cmd_argument);

  /**
   * @brief Gets the ID of this runner
   * @return the ID of this runner
   */
  uint32_t     GetId();

  /**
   * @brief Gets the name of this runner that is displayed in the GUI
   * @return the name of this runner
   */
  std::string  GetName();

  /**
   * @brief Gets the path to the runner's executable
   * @return the path to the runner's executable
   */
  std::string  GetPath();

  /**
   * @brief Gets the default path to the directory where the .dll / .so / etc. files are located
   * @return the default algorithm directory
   */
  std::string  GetDefaultAlgoDir();

  /**
   * @brief Gets the command line argument for loading files
   * @return the command line argument for loading files
   */
  std::string  GetLoadCmdArgument();

private:
  std::mutex   m_mutex;                   /**< Mutex for thread-safe getting and setting of all values */

  uint32_t     m_id;                      /**< ID of this runner when saving it to a file */
  std::string  m_name;                    /**< Name of this runner that is displayed in the GUI */
  std::string  m_path;                    /**< Path to the runner's executable */
  std::string  m_default_algo_dir;        /**< Default path to the directory where the .dll / .so / etc. files are located */
  std::string  m_load_cmd_argument;       /**< command line argument for loading a file */
};

