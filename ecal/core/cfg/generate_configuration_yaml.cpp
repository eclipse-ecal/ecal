/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
 *         Use with no arguments to create the eCAL directory in the first available of the following paths:
 *         1. ECAL_DATA environment variable path if set
 *         2. local user path
 *         3. system path like etc, ProgramData
**/

#include "config/configuration_writer.h"
#include "config/default_configuration.h"
#include "config/ecal_path_processing.h"

#include <iostream>
#include <string>

void showHelp() {
#ifdef ECAL_OS_WINDOWS
  const std::string program_name = "ecal_generate_config.exe";
#else
  const std::string program_name = "ecal_generate_config";
#endif

  std::cout << "Usage: " << program_name << " [OPTIONS]\n\n"
            << "Options:\n"
            << "  -d, --dump            Write the default configuration to file in the current folder\n"
            << "  -h, --help            Show this help message and exit\n\n"
            << "Description:\n"
            << "  Generate an ecal.yaml file with default configuration.\n"
            << "  Use --dump or -d to write the default configuration to file in the current folder.\n"
            << "  Use with no arguments to create the eCAL directory in the first available of the following paths:\n"
            << "    1. ECAL_DATA environment variable path if set\n"
            << "    2. Local user path (/home/[username] ,  C:\\Users\\[username]\\AppData\\Local)\n"
            << "    3. System path (etc, C:\\ProgramData)\n\n"
            << "Examples:\n"
            << "  " << program_name << " --dump\n"
            << "  " << program_name << "\n";
}


void exitWithMessage(const std::string& message, int exitCode, bool wait_for_it = true) {
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
  bool dump = false;

  for (int i = 1; i < argc; i++) {
    const std::string arg = argv[i];
    if (arg == "--dump" || arg == "-d") {
      dump = true;
    } 
    else if (arg == "--help" || arg == "-h") 
    {
      showHelp();
      exit(0);
    }
    else 
    {
      showHelp();
      exitWithMessage("Unknown argument: " + arg, 1);
    }
  }

  if (dump) {
    if (!eCAL::Config::dumpConfigToFile()) {
      exitWithMessage("Failed to write default configuration to file.", 1, false);
    }

    if (!eCAL::Config::dumpToFile(eCAL::Config::getTimeConfigAsYamlSS(), "ecaltime.yaml")) {
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
    if (!eCAL::Config::dumpDefaultConfig(created_path)) {
      exitWithMessage("Failed to write default configuration to file.", 1);
    }
    if (!eCAL::Config::dumpToFile(eCAL::Config::getTimeConfigAsYamlSS(), created_path + "/ecaltime.yaml")) {
      exitWithMessage("Failed to write time configuration to file.", 1);
    }

    exitWithMessage("Created eCAL configuration files in: " + created_path, 0);
  } else {
    exitWithMessage("Failed to create eCAL directory structure.", 1);
  }
}