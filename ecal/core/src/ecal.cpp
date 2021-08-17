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

/**
 * @brief  eCAL core functions
**/

#include "ecal_globals.h"

#include <tclap/CmdLine.h>
#include <custom_tclap/advanced_tclap_output.h>

namespace eCAL
{
 /**
   * @brief  Get eCAL version string. 
   *
   * @return  Full eCAL version string. 
  **/
  const char* GetVersionString()
  {
    return(ECAL_VERSION);
  }

  /**
   * @brief  Get eCAL version date. 
   *
   * @return  Full eCAL version date string. 
  **/
  const char* GetVersionDateString()
  {
    return(ECAL_DATE);
  }

  /**
   * @brief  Get eCAL version as separated integer values. 
   *
   * @param [out] major_  The eCAL major version number.
   * @param [out] minor_  The eCAL minor version number.
   * @param [out] patch_  The eCAL patch version number.
   *
   * @return  Zero if succeeded.
  **/
  int GetVersion(int* major_, int* minor_, int* patch_)
  {
    if((major_ == nullptr) && (minor_ == nullptr) && (patch_ == nullptr)) return(-1);
    if(major_) *major_ = ECAL_VERSION_MAJOR;
    if(minor_) *minor_ = ECAL_VERSION_MINOR;
    if(patch_) *patch_ = ECAL_VERSION_PATCH;
    return(0);
  }

  /**
   * @brief Initialize eCAL API.
   *
   * @param argc_        Number of command line arguments. 
   * @param argv_        Array of command line arguments. 
   * @param unit_name_   Defines the name of the eCAL unit. 
   * @param components_  Defines which component to initialize.
   *
   * @return Zero if succeeded, 1 if already initialized, -1 if failed.
  **/
  int Initialize(int argc_ , char **argv_, const char *unit_name_, unsigned int components_)
  {
    bool dump_config(false);
    std::vector<std::string> config_keys;
    if ((argc_ > 0) && (argv_ != nullptr))
    {
      // define command line object
      TCLAP::CmdLine cmd("", ' ', ECAL_VERSION);

      // define command line arguments
      TCLAP::SwitchArg             dump_config_arg     ("", "ecal-dump-config",    "Dump current configuration.", false);
      TCLAP::ValueArg<std::string> default_ini_file_arg("", "ecal-ini-file",       "Load default configuration from that file.", false, ECAL_DEFAULT_CFG, "string");
      TCLAP::MultiArg<std::string> set_config_key_arg  ("", "ecal-set-config-key", "Overwrite a specific configuration key (ecal-set-config-key \"section/key:value\".", false, "string");

      TCLAP::UnlabeledMultiArg<std::string> dummy_arg("__dummy__", "Dummy", false, ""); // Dummy arg to eat all unrecognized arguments

      cmd.add(dump_config_arg);
      cmd.add(default_ini_file_arg);
      cmd.add(set_config_key_arg);
      cmd.add(dummy_arg);

      CustomTclap::AdvancedTclapOutput advanced_tclap_output(&std::cout, 75);
      advanced_tclap_output.setArgumentHidden(&dummy_arg, true);
      cmd.setOutput(&advanced_tclap_output);

      // parse command line
      cmd.parse(argc_, argv_);

      // set globals
      if (dump_config_arg.isSet())
      {
        dump_config = true;
      }
      if (default_ini_file_arg.isSet())
      {
        g_default_ini_file = default_ini_file_arg.getValue();
      }
      if (set_config_key_arg.isSet())
      {
        config_keys = set_config_key_arg.getValue();
      }
    }

    // first call
    if (g_globals_ctx == nullptr)
    {
      g_globals_ctx = new CGlobals;

      if(unit_name_) g_unit_name = unit_name_;
      if (g_unit_name.empty())
      {
        g_unit_name = Process::GetProcessName();
#ifdef ECAL_OS_WINDOWS
        size_t p = g_unit_name.rfind('\\');
        if (p != std::string::npos)
        {
          g_unit_name = g_unit_name.substr(p+1);
        }
        p = g_unit_name.rfind('.');
        if (p != std::string::npos)
        {
          g_unit_name = g_unit_name.substr(0, p);
        }
#endif
#ifdef ECAL_OS_LINUX
        size_t p = g_unit_name.rfind('/');
        if (p != std::string::npos)
        {
          g_unit_name = g_unit_name.substr(p + 1);
        }
#endif
      }

      if (argv_)
      {
        for (size_t i = 0; i < static_cast<size_t>(argc_); ++i) if (argv_[i]) g_task_parameter.push_back(argv_[i]);
      }

      g_process_wclock = 0;
      g_process_wbytes = 0;
      g_process_wbytes_sum = 0;

      g_process_rclock = 0;
      g_process_rbytes = 0;
      g_process_rbytes_sum = 0;
    }
    g_globals_ctx_ref_cnt++;

    // (post)initialize single components
    int success = g_globals()->Initialize(components_, &config_keys);

    // print out configuration
    if (dump_config)
    {
      Process::DumpConfig();
    }

    return success;
  }

  /**
   * @brief Initialize eCAL API.
   *
   * @param args_        Vector of config arguments to overwrite (["arg1", "value1", "arg2", "arg3", "value3" ..]).
   * @param unit_name_   Defines the name of the eCAL unit.
   * @param components_  Defines which component to initialize.
   *
   * @return Zero if succeeded, 1 if already initialized, -1 if failed.
  **/
  int Initialize(std::vector<std::string> args_, const char *unit_name_, unsigned int components_) //-V826
  {
    args_.emplace(args_.begin(), eCAL::Process::GetProcessName());
    std::vector<const char*> argv(args_.size());
    std::transform(args_.begin(), args_.end(), argv.begin(), [](std::string& s) {return s.c_str();});
    return Initialize(static_cast<int>(argv.size()), const_cast<char**>(argv.data()), unit_name_, components_);
  }

  /**
   * @brief Check eCAL initialize state.
   *
   * @param component_  Check specific component or 0 for general state of eCAL core.
   *
   * @return True if eCAL is initialized.
  **/
  int IsInitialized(unsigned int component_)
  {
    if (g_globals_ctx == nullptr) return(0);
    return(g_globals()->IsInitialized(component_));
  }

  /**
   * @brief  Set/change the unit name of current module.
   *
   * @param unit_name_  Defines the name of the eCAL unit. 
   *
   * @return  Zero if succeeded.
  **/
  int SetUnitName(const char *unit_name_)
  {
    g_unit_name = unit_name_;
    return(0);
  }

  /**
   * @brief Finalize eCAL API.
   *
   * @param components_  Defines which component to finalize.
   *
   * @return Zero if succeeded, 1 if already finalized, -1 if failed.
  **/
  int Finalize(unsigned int components_)
  {
    if (g_globals_ctx == nullptr) return 1;
    g_globals_ctx_ref_cnt--;
    if (g_globals_ctx_ref_cnt > 0) return 0;
    int ret = g_globals()->Finalize(components_);
    delete g_globals_ctx;
    g_globals_ctx = nullptr;
    return(ret);
  }
}
