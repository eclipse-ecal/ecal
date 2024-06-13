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
  // to remove deprecated warning
  #define getcwd _getcwd
#endif
#ifdef ECAL_OS_LINUX
  #include <unistd.h>
#endif

namespace
{
  // copied and adapted from ecal_config_reader.cpp
#ifdef ECAL_OS_WINDOWS
  const char path_separator('\\');
#endif /* ECAL_OS_WINDOWS */
#ifdef ECAL_OS_LINUX
  const char path_separator('/');
#endif /* ECAL_OS_LINUX */

  bool setPathSep(std::string& file_path_)
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
    setPathSep(ecal_data_path);
    return ecal_data_path;
  }

  std::string cwdPath()
  {
    std::string cwd_path = { getcwd(nullptr, 0) };
    
    if (cwd_path.empty())
      throw std::runtime_error("getcwd() : cannot read current working directory.");
        
    setPathSep(cwd_path);
    return cwd_path;
  }

  std::string eCALDataCMakePath()
  {
    std::string cmake_data_path;
#ifdef ECAL_OS_LINUX
    const std::string ecal_install_config_dir(ECAL_INSTALL_CONFIG_DIR);
    const std::string ecal_install_prefix(ECAL_INSTALL_PREFIX);

    if ((!ecal_install_config_dir.empty() && (ecal_install_config_dir[0] == path_separator))
      || ecal_install_prefix.empty())
    {
      cmake_data_path = ecal_install_config_dir;
    }
    else if (!ecal_install_prefix.empty())
    {
      cmake_data_path = ecal_install_prefix + path_separator + ecal_install_config_dir;
    }
    setPathSep(cmake_data_path);
#endif /* ECAL_OS_LINUX */  
    return cmake_data_path;
  }

  std::string eCALDataSystemPath()
  {
    std::string system_data_path;
#ifdef ECAL_OS_WINDOWS
    system_data_path = getEnvVar("ProgramData");
    if(setPathSep(system_data_path))
    {
        system_data_path += std::string("eCAL");
        setPathSep(system_data_path);
    }
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX
    system_data_path = "/etc/ecal";
    setPathSep(system_data_path);
#endif /* ECAL_OS_LINUX */
    return system_data_path;
  }

  void appendFileNameToPathIfPathIsValid(std::string& path_, const std::string& file_name_)
  {
    if (!path_.empty())
      path_ += file_name_;
  }

  void parseConfigKeysToMap(const std::vector<std::string>& config_keys_, eCAL::Cli::ConfigKey2DMap& map_)
  {
    // each string has the format "section/key:value"
    for (const auto& full_key : config_keys_)
    {
        auto sec_pos = full_key.find_last_of('/');
        if (sec_pos == std::string::npos) continue;
        const std::string section = full_key.substr(0, sec_pos);
        std::string key     = full_key.substr(sec_pos+1);

        auto val_pos = key.find_first_of(':');
        if (val_pos == std::string::npos) continue;
        const std::string value = key.substr(val_pos+1);
        key = key.substr(0, val_pos);

        map_[section][key] = value;
    }
  }

  bool isValidConfigFilePath(const std::string& file_path_)
  {
    // check existence of user defined file
    const EcalUtils::Filesystem::FileStatus ecal_ini_status(file_path_, EcalUtils::Filesystem::Current);
    return ecal_ini_status.IsOk() && (ecal_ini_status.GetType() == EcalUtils::Filesystem::Type::RegularFile);
  }

  std::string checkForValidConfigFilePath(const std::string& config_file_)
    {
      // differences to ecal_config_reader implementation are:
      //    1. does not use the default ini file name, instead uses the specified file
      //    2. searches relative to the executable path and takes it as highest priority
      //    3. throws a runtime error, if it cannot find the specified file

      // -----------------------------------------------------------
      // precedence 1: relative path to executable
      // -----------------------------------------------------------
      std::string cwd_directory_path = cwdPath();
      appendFileNameToPathIfPathIsValid(cwd_directory_path, config_file_);

      // -----------------------------------------------------------
      // precedence 2: ECAL_DATA variable (windows and linux)
      // -----------------------------------------------------------
      std::string ecal_data_path = eCALDataEnvPath();
      appendFileNameToPathIfPathIsValid(ecal_data_path, config_file_);
      
      // -----------------------------------------------------------
      // precedence 3:  cmake configured data paths (linux only)
      // -----------------------------------------------------------
      std::string cmake_data_path = eCALDataCMakePath();
      appendFileNameToPathIfPathIsValid(cmake_data_path, config_file_);

      // -----------------------------------------------------------
      // precedence 4: system data path 
      // -----------------------------------------------------------
      std::string system_data_path = eCALDataSystemPath();
      appendFileNameToPathIfPathIsValid(system_data_path, config_file_);

      // Check for first directory which contains the ini file.
      std::vector<std::string> search_directories{ cwd_directory_path, ecal_data_path, cmake_data_path, system_data_path };

      auto it = std::find_if(search_directories.begin(), search_directories.end(), isValidConfigFilePath);
      // We should have encountered a valid path
      if (it != search_directories.end())
        return (*it);

      // Check if user specified complete path, in case all other precedence paths exist
      if (isValidConfigFilePath(config_file_))
      {
        return std::string(config_file_);
      }

      // If valid path is not encountered, throw error
      throw std::runtime_error("[CMD Parser] Specified config file: \"" + config_file_ + "\" not found.");
    }
}

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
        cmd.parse(arguments_);

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
          parseConfigKeysToMap(set_config_key_arg.getValue(), m_config_key_map);
        }
      }
#endif  
      m_task_parameter = arguments_;
    }

    bool CmdParser::getDumpConfig() const                        { return m_dump_config; };
    std::vector<std::string>& CmdParser::getConfigKeys()         { return m_config_keys; };
    std::vector<std::string>& CmdParser::getTaskParameter()      { return m_task_parameter; };
    std::string& CmdParser::getUserIni()                         { return m_user_ini; };
    Cli::ConfigKey2DMap& CmdParser::getConfigKeysMap()           { return m_config_key_map; };  
  }
}
