/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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
 * @brief  Generate an ecal.yaml file with default configuration.
 *         Use --dump or -d to write the default configuration to file in current folder.
 *         Use --overwrite or -o to overwrite existing configuration files without asking.
 *         Use --no-wait or -n to exit without waiting for user input (useful for scripts).
 *         Use with no arguments to create the eCAL directory in the first available of the following paths:
 *         1. ECAL_DATA environment variable path if set
 *         2. local user path
 *         3. system path like etc, ProgramData
**/

#include "config/configuration_writer.h"
#include "config/default_configuration.h"
#include "config/ecal_path_processing.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

void showHelp(std::string_view argv0) {
  const auto program_name = std::filesystem::path(argv0).filename();

  std::cout << "Usage: " << program_name << " [OPTIONS]\n\n"
            << "Options:\n"
            << "  -d, --dump            Write the default configuration to file in the current folder\n"
            << "  -o, --overwrite       Overwrite existing configuration files without asking\n"
            << "  -n, --no-wait         Exit without waiting for user input (e.g. for use in scripts)\n"
            << "  -h, --help            Show this help message and exit\n\n"
            << "Description:\n"
            << "  Generate an ecal.yaml file with default configuration.\n"
            << "  Use --dump or -d to write the default configuration to file in the current folder.\n"
            << "  Use --overwrite or -o to skip the overwrite confirmation when files already exist.\n"
            << "  Use --no-wait or -n to suppress the 'Press Enter to continue...' prompt.\n"
            << "  Use with no arguments to create the eCAL directory in the first available of the following paths:\n"
            << "    1. ECAL_DATA environment variable path if set\n"
            << "    2. Local user path (/home/[username] ,  C:\\Users\\[username]\\AppData\\Local)\n"
            << "    3. System path (etc, C:\\ProgramData)\n\n"
            << "Examples:\n"
            << "  " << program_name << " --dump\n"
            << "  " << program_name << " --dump --overwrite --no-wait\n"
            << "  " << program_name << "\n";
}


// Returns true if writing should proceed.
// If overwrite is true, always proceeds. Otherwise, if any of the listed files
// already exist, asks the user interactively whether to overwrite them.
[[nodiscard]] static bool handleExistingFiles(const std::vector<std::filesystem::path>& file_paths, bool overwrite)
{
  if (overwrite) return true;

  std::vector<std::filesystem::path> existing;
  for (const auto& path : file_paths)
  {
    if (std::filesystem::exists(path))
      existing.push_back(path);
  }

  if (existing.empty()) return true;

  std::cout << "The following configuration file(s) already exist:\n";
  for (const auto& path : existing)
    std::cout << "  " << path.string() << "\n";

  std::cout << "Overwrite? [y/N]: ";
  std::string answer;
  std::getline(std::cin, answer);
  return (!answer.empty() && (answer[0] == 'y' || answer[0] == 'Y'));
}

[[noreturn]] void exitWithMessage(std::string_view message, int exitCode, bool wait_for_it = true) {
  if (!message.empty()) {
    if (exitCode == 0) {
      std::cout << message << "\n";
    } else {
      std::cerr << message << "\n";
    }
  }
  
  if (wait_for_it)
  {
    std::cout << "Press Enter to continue...";
    std::cin.get();
  }

  exit(exitCode);
}

int main(int argc, char* argv[]) {
  bool dump      = false;
  bool overwrite = false;
  bool no_wait   = false;

  for (int i = 1; i < argc; i++) {
    const std::string_view arg = argv[i];
    if (arg == "--dump" || arg == "-d") {
      dump = true;
    }
    else if (arg == "--overwrite" || arg == "-o")
    {
      overwrite = true;
    }
    else if (arg == "--no-wait" || arg == "-n")
    {
      no_wait = true;
    }
    else if (arg == "--help" || arg == "-h") 
    {
      showHelp(argv[0]);
      exit(0);
    }
    else 
    {
      showHelp(argv[0]);
      exitWithMessage("Unknown argument: " + std::string(arg), 1, !no_wait);
    }
  }

  if (dump) {
    const std::filesystem::path ecal_cfg_path = "ecal.yaml";
    const std::filesystem::path time_cfg_path = "ecaltime.yaml";

    if (!handleExistingFiles({ecal_cfg_path, time_cfg_path}, overwrite)) {
      exitWithMessage("Aborted. Existing configuration files were not overwritten.", 0, !no_wait);
    }

    if (!eCAL::Config::dumpConfigToFile()) {
      exitWithMessage("Failed to write default configuration to file.", 1, false);
    }

    if (!eCAL::Config::dumpToFile(eCAL::Config::getTimeConfigAsYamlSS(), time_cfg_path.string())) {
      exitWithMessage("Failed to write time configuration to file.", 1, false);
    }

    exitWithMessage("", 0, false);
  }

  std::string created_path;
  auto dir_manager = eCAL::Util::DirManager();
  const std::vector<std::string> dir_paths = eCAL::Config::getEcalDefaultPaths(eCAL::Util::DirProvider(), dir_manager);

  // create the ecal paths
  for (const std::string& path : dir_paths) {
    if (!path.empty() && dir_manager.createEcalDirStructure(path)) {
      created_path = path;
      std::cout << "Created eCAL directory structure in: " << path << "\n";
      break;
    }
  }

  // dump config to file
  if (!created_path.empty()) {
    const std::filesystem::path base         = created_path;
    const std::filesystem::path ecal_cfg_path = base / "ecal.yaml";
    const std::filesystem::path time_cfg_path = base / "ecaltime.yaml";

    if (!handleExistingFiles({ecal_cfg_path, time_cfg_path}, overwrite)) {
      exitWithMessage("Aborted. Existing configuration files were not overwritten.", 0, !no_wait);
    }

    if (!eCAL::Config::dumpDefaultConfig(created_path)) {
      exitWithMessage("Failed to write default configuration to file.", 1, !no_wait);
    }
    if (!eCAL::Config::dumpToFile(eCAL::Config::getTimeConfigAsYamlSS(), time_cfg_path.string())) {
      exitWithMessage("Failed to write time configuration to file.", 1, !no_wait);
    }

    exitWithMessage("Created eCAL configuration files in: " + created_path, 0, !no_wait);
  } else {
    exitWithMessage("Failed to create eCAL directory structure.", 1, !no_wait);
  }
}