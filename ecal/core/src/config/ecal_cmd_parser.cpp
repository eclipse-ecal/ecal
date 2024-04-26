#include "config/ecal_cmd_parser.h"

#include "ecal/ecal_defs.h"
#include "ecal_def.h"
#include "ecal_utils/filesystem.h"
#include "util/getenvvar.h"

#if ECAL_CORE_COMMAND_LINE
  #include "util/advanced_tclap_output.h"
#endif

#include <algorithm>

// for cwd
#ifdef ECAL_OS_WINDOWS
  #include <direct.h>
  // to get rid of deprecated warning
  #define getcwd _getcwd
#endif
#ifdef ECAL_OS_LINUX
  #include <unistd.h>
#endif

namespace
{
  // things stolen and adapted from ecal_config_reader.cpp
#ifdef ECAL_OS_WINDOWS
  const char path_separator('\\');
#endif /* ECAL_OS_WINDOWS */
#ifdef ECAL_OS_LINUX
  const char path_separator('/');
#endif /* ECAL_OS_LINUX */

  bool SetPathSep(std::string file_path_)
  {
    if (!file_path_.empty())
    {
      if (file_path_.back() != path_separator)
      {
        file_path_ += path_separator;
      }
      return true;
    }

    return false;
  }

  std::string eCALDataEnvPath()
  {
    std::string ecal_data_path = getEnvVar("ECAL_DATA");
    SetPathSep(ecal_data_path);
    return ecal_data_path;
  }

  std::string cwdPath()
  {
    std::string cwd_path = {};
    char* buffer;

    if ( (buffer = getcwd(NULL, 0)) == NULL )
      throw std::runtime_error("getcwd() : cannot read current working directory.");
    else
    {
      cwd_path = std::string(buffer);
      free(buffer);
    }
    
    SetPathSep(cwd_path);
    return cwd_path;
  }

  std::string eCALDataCMakePath()
  {
    std::string cmake_data_path;
#ifdef ECAL_OS_LINUX
    std::string ecal_install_config_dir(ECAL_INSTALL_CONFIG_DIR);
    std::string ecal_install_prefix(ECAL_INSTALL_PREFIX);

    if ((!ecal_install_config_dir.empty() && (ecal_install_config_dir[0] == path_separator))
      || ecal_install_prefix.empty())
    {
      cmake_data_path = ecal_install_config_dir;
    }
    else if (!ecal_install_prefix.empty())
    {
      cmake_data_path = ecal_install_prefix + path_separator + ecal_install_config_dir;
    }
    SetPathSep(cmake_data_path);
#endif /* ECAL_OS_LINUX */  
    return cmake_data_path;
  }

  std::string eCALDataSystemPath()
  {
    std::string system_data_path;
#ifdef ECAL_OS_WINDOWS
    system_data_path = getEnvVar("ProgramData");
    if(SetPathSep(system_data_path))
        system_data_path += std::string("eCAL") + path_separator;
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX
    system_data_path = "/etc/ecal/";
#endif /* ECAL_OS_LINUX */
    return system_data_path;
  }

  bool isValidConfigFile(std::string file_path_)
  {
    if (file_path_.empty()) { return false; }

    // check existence of user defined file
    const EcalUtils::Filesystem::FileStatus ecal_ini_status(file_path_, EcalUtils::Filesystem::Current);
    if (ecal_ini_status.IsOk() && (ecal_ini_status.GetType() == EcalUtils::Filesystem::Type::RegularFile))
    {
      return true;
    }

    return false;
  }
}

namespace eCAL
{
  namespace Config
  {
    CmdParser::CmdParser() 
    : m_dump_config{false}
    , m_config_keys{}
    , m_task_parameter{}
    , m_user_ini{} 
    {}

    CmdParser::CmdParser(int argc_ , char **argv_)
    : CmdParser()
    {
      parseArguments(argc_, argv_);
    }

    void CmdParser::parseArguments(int argc_, char **argv_)
    {
#if ECAL_CORE_COMMAND_LINE
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
          m_dump_config = true;
        }
        if (default_ini_file_arg.isSet())
        {
          m_user_ini = checkForValidConfigFilePath(default_ini_file_arg.getValue());
        }
        if (set_config_key_arg.isSet())
        {
          m_config_keys = set_config_key_arg.getValue();
        }
      }
#endif  
      if (argv_ != nullptr)
      {
        for (size_t i = 0; i < static_cast<size_t>(argc_); ++i) if (argv_[i] != nullptr) m_task_parameter.emplace_back(argv_[i]);
      }

    }

    std::string CmdParser::checkForValidConfigFilePath(std::string config_file_)
    {
      // differences to ecal_config_reader implementation are:
      //    1. it does not use the default ini file name, instead uses the specified file
      //    2. it searches relative to the executable path and takes it as highest priority
      //    3. it throws a runtime error, if it cannot find the specified file

      // -----------------------------------------------------------
      // precedence 1: relative path to executable
      // -----------------------------------------------------------
      const std::string cwd_directory_path{ cwdPath() + config_file_};
      // -----------------------------------------------------------
      // precedence 2: ECAL_DATA variable (windows and linux)
      // -----------------------------------------------------------
      const std::string ecal_data_path{ eCALDataEnvPath() + config_file_};
      
      // -----------------------------------------------------------
      // precedence 3:  cmake configured data paths (linux only)
      // -----------------------------------------------------------
      const std::string cmake_data_path{ eCALDataCMakePath() + config_file_};

      // -----------------------------------------------------------
      // precedence 4: system data path 
      // -----------------------------------------------------------
      const std::string system_data_path(eCALDataSystemPath() + config_file_);

      // Check for first directory which contains the ini file.
      std::vector<std::string> search_directories{ cwd_directory_path, ecal_data_path, cmake_data_path, system_data_path };

      auto it = std::find_if(search_directories.begin(), search_directories.end(), isValidConfigFile);
      // We should have encountered a valid path
      if (it != search_directories.end())
        return (*it);

      // If valid path is not encountered, throw error
      throw std::runtime_error("[CMD Parser] Specified config file: \"" + config_file_ + "\" not found.");
    }

    bool CmdParser::getDumpConfig() const                        { return m_dump_config; };
    std::vector<std::string>& CmdParser::getConfigKeys()         { return m_config_keys; };
    std::vector<std::string>& CmdParser::getTaskParameter()      { return m_task_parameter; };
    std::string& CmdParser::getUserIni()                         { return m_user_ini; };
  }
}
