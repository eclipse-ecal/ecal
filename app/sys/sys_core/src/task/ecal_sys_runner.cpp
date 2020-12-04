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

#include "ecalsys/task/ecal_sys_runner.h"

#include <random>
#include <limits.h>

EcalSysRunner::EcalSysRunner()
  : m_id               (0)
  , m_name             ("")
  , m_path             ("")
  , m_default_algo_dir ("")
  , m_load_cmd_argument("")
{}


EcalSysRunner::EcalSysRunner(uint32_t id, const std::string& name, const std::string& path, const std::string& default_algo_dir, const std::string& load_cmd_argument)
  : m_id               (id)
  , m_name             (name)
  , m_path             (path)
  , m_default_algo_dir (default_algo_dir)
  , m_load_cmd_argument(load_cmd_argument)
{}

void EcalSysRunner::SetId(uint32_t id) {
  std::lock_guard<std::mutex> runner_lock(m_mutex);
  m_id = id;
}

void EcalSysRunner::SetName(const std::string& name) {
  std::lock_guard<std::mutex> runner_lock(m_mutex);
  m_name = name;
}

void EcalSysRunner::SetPath(const std::string& path) {
  std::lock_guard<std::mutex> runner_lock(m_mutex);
  m_path = path;
}

void EcalSysRunner::SetDefaultAlgoDir(const std::string& default_algo_dir) {
  std::lock_guard<std::mutex> runner_lock(m_mutex);
  m_default_algo_dir = default_algo_dir;
}

void EcalSysRunner::SetLoadCmdArgument(const std::string& load_cmd_argument) {
  std::lock_guard<std::mutex> runner_lock(m_mutex);
  m_load_cmd_argument = load_cmd_argument;
}

uint32_t EcalSysRunner::GetId() {
  std::lock_guard<std::mutex> runner_lock(m_mutex);
  return m_id;
}

std::string EcalSysRunner::GetName() {
  std::lock_guard<std::mutex> runner_lock(m_mutex);
  return m_name;
}

std::string EcalSysRunner::GetPath() {
  std::lock_guard<std::mutex> runner_lock(m_mutex);
  return m_path;
}

std::string EcalSysRunner::GetDefaultAlgoDir() {
  std::lock_guard<std::mutex> runner_lock(m_mutex);
  return m_default_algo_dir;
}

std::string EcalSysRunner::GetLoadCmdArgument() {
  std::lock_guard<std::mutex> runner_lock(m_mutex);
  return m_load_cmd_argument;
}

EcalSysRunner::~EcalSysRunner()
{}
