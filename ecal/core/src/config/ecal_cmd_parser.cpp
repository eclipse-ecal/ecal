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

#include "config/ecal_cmd_parser.h"

#include "ecal/ecal_defs.h"
#include "ecal_def.h"

#if ECAL_CORE_COMMAND_LINE
  #include "util/advanced_tclap_output.h"
#endif

#include <algorithm>


namespace eCAL
{
  namespace Config
  {
    CmdParser::CmdParser() 
    : m_dump_config{false}
    {}

    CmdParser::CmdParser(std::vector<std::string>& arguments_)
    : CmdParser()
    {
      parseArguments(arguments_);
    }

    void CmdParser::parseArguments(std::vector<std::string>& arguments_)
    {
#if ECAL_CORE_COMMAND_LINE
      if (!arguments_.empty())
      {
        // define command line object
        TCLAP::CmdLine cmd("", ' ', ECAL_VERSION);

        // define command line arguments
        TCLAP::SwitchArg             dump_config_arg     ("", "ecal-dump-config",    "Dump current configuration.", false);
        TCLAP::ValueArg<std::string> default_ini_file_arg("", "ecal-ini-file",       "Load default configuration from that file.", false, ECAL_DEFAULT_CFG, "string");

        TCLAP::UnlabeledMultiArg<std::string> dummy_arg("__dummy__", "Dummy", false, ""); // Dummy arg to eat all unrecognized arguments

        cmd.add(dump_config_arg);
        cmd.add(default_ini_file_arg);
        cmd.add(dummy_arg);

        CustomTclap::AdvancedTclapOutput advanced_tclap_output(&std::cout, 75);
        advanced_tclap_output.setArgumentHidden(&dummy_arg, true);
        cmd.setOutput(&advanced_tclap_output);

        // parse command line
        cmd.parse(arguments_);

        // set globals
        if (dump_config_arg.isSet())
        {
          m_dump_config = true;
        }
        if (default_ini_file_arg.isSet())
        {
          m_user_ini = default_ini_file_arg.getValue();
        }
      }
#endif  
    }

    bool CmdParser::getDumpConfig() const                        { return m_dump_config; };
    std::string& CmdParser::getUserIni()                         { return m_user_ini; };
  }
}
