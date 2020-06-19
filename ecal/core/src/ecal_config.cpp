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
 * @brief  Global config class
**/

#include <ecal/ecal_os.h>

#include "ecal_def.h"
#include "ecal_config.h"
#include "ecal_global_accessors.h"
#include "getenvvar.h"

#include <iostream>
#include <fstream>

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_main.h"
#endif

#ifdef ECAL_OS_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#endif

#include <SimpleIni.h>

namespace
{
  bool fileexists(const std::string& fname_)
  {
    std::ifstream infile(fname_);
    return infile.good();
  }

  bool direxists(const std::string& path_)
  {
#ifdef ECAL_OS_WINDOWS
    DWORD dwAttrib = GetFileAttributes(path_.c_str());
    return(dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
#ifdef ECAL_OS_LINUX
    struct stat sb;
    return(stat(path_.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode));
#endif
  }

  void createdir(const std::string& path_)
  {
#ifdef ECAL_OS_WINDOWS
    CreateDirectoryA(path_.c_str(), NULL);
#else
    mkdir(path_.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
  }
}

namespace eCAL
{
  namespace Util
  {
    ECAL_API std::string GeteCALHomePath()
    {
      std::string home_path;

#ifdef ECAL_OS_WINDOWS
      // check ECAL_HOME
      home_path = getEnvVar("ECAL_HOME");
      if (!home_path.empty())
      {
        if (*home_path.rbegin() != '\\') home_path += '\\';
      }
      if (!std::string(ECAL_HOME_PATH_WINDOWS).empty()) //-V815
      {
        home_path += "\\";
        home_path += ECAL_HOME_PATH_WINDOWS;
      }
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX
      const char *hdir;
      if ((hdir = getenv("HOME")) == NULL) {
        hdir = getpwuid(getuid())->pw_dir;
      }
      home_path += hdir;
      if (!std::string(ECAL_HOME_PATH_LINUX).empty())
      {
        home_path += "/";
        home_path += ECAL_HOME_PATH_LINUX;
      }
#endif /* ECAL_OS_LINUX */

      // create if not exists
      if (!direxists(home_path))
      {
        createdir(home_path);
      }

#ifdef ECAL_OS_WINDOWS
      home_path += "\\";
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX
      home_path += '/';
#endif /* ECAL_OS_LINUX */

      return(home_path);
    }

    ECAL_API std::string GeteCALConfigPath()
    {
      std::string config_path;

#ifdef ECAL_OS_WINDOWS
      // check ECAL_DATA
      config_path = getEnvVar("ECAL_DATA");
      if (!config_path.empty())
      {
        if (*config_path.rbegin() != '\\') config_path += '\\';
        return(config_path);
      }

      config_path = getEnvVar("ProgramData");
      if (!config_path.empty())
      {
        if (*config_path.rbegin() != '\\') config_path += '\\';
      }

      config_path += "eCAL";
      if (*config_path.rbegin() != '\\') config_path += '\\';
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX
      config_path = ECAL_USER_CONFIG_PATH_LINUX;
      if (!direxists(config_path))
      {
        config_path = ECAL_CONFIG_PATH_LINUX;
      }
#endif /* ECAL_OS_LINUX */

      return(config_path);
    }

    ECAL_API std::string GeteCALUserSettingsPath()
    {
      std::string settings_path;
#ifdef ECAL_OS_WINDOWS
      settings_path = GeteCALConfigPath();
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX
      settings_path = GeteCALHomePath();
#endif /* ECAL_OS_LINUX */
      settings_path += std::string(ECAL_SETTINGS_PATH);

      if (!direxists(settings_path))
      {
        createdir(settings_path);
      }

#ifdef ECAL_OS_WINDOWS
      settings_path += "\\";
#else
      settings_path += "/";
#endif

      return(settings_path);
    }

    ECAL_API std::string GeteCALLogPath()
    {
      std::string log_path;
#ifdef ECAL_OS_WINDOWS
      log_path = GeteCALConfigPath();
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX
      log_path = GeteCALHomePath();
#endif /* ECAL_OS_LINUX */

      log_path += std::string(ECAL_LOG_PATH);

      if (!direxists(log_path))
      {
        createdir(log_path);
      }

#ifdef ECAL_OS_WINDOWS
      log_path += "\\";
#else
      log_path += "/";
#endif

      return(log_path);
    }

    ECAL_API std::string GeteCALDefaultIniFile()
    {
      std::string ini_file = GeteCALConfigPath();
      ini_file += ECAL_DEFAULT_CFG;
      return ini_file;
    }
  }

  ////////////////////////////////////////////////////////
  // CConfigImpl
  ////////////////////////////////////////////////////////
  class CConfigImpl : public CSimpleIni
  {
  public:
    CConfigImpl()
    {
    }

    virtual ~CConfigImpl()
    {
    }

    void OverwriteKeys(const std::vector<std::string>& key_vec_)
    {
      m_overwrite_keys = key_vec_;
    }

    void AddFile(std::string& file_name_)
    {
      std::string cfg_fname = file_name_;
      if (!fileexists(cfg_fname))
      {
        cfg_fname = Util::GeteCALConfigPath() + cfg_fname;
      }

      // load settings config file
      bool loaded(false);
      if (fileexists(cfg_fname))
      {
        // apply file name to manager
        loaded = LoadFile(cfg_fname.c_str()) == 0;        
      }

      // load error ?
      if(!loaded)
      { 
        std::cout << "eCAL::Config - Could not load ini file, using defaults. File name : " << cfg_fname << std::endl;
      }
      else
      {
        // return full name
        file_name_ = cfg_fname;
      }

      // update command line keys
      for (auto full_key : m_overwrite_keys)
      {
         auto sec_pos = full_key.find_last_of('/');
         if (sec_pos == std::string::npos) continue;
         std::string section = full_key.substr(0, sec_pos);
         std::string key     = full_key.substr(sec_pos+1);

         auto val_pos = key.find_first_of(':');
         if (val_pos == std::string::npos) continue;
         std::string value = key.substr(val_pos+1);
         key = key.substr(0, val_pos);

         SI_Error err = SetValue(section.c_str(), key.c_str(), value.c_str());
         if (err == SI_FAIL)
         {
            std::cout << "Error: Could not overwrite key " << key << " in section " << section << ".";
         }
      }
    }
  protected:
    std::vector<std::string> m_overwrite_keys;
  };

  ////////////////////////////////////////////////////////
  // CConfigBase
  ////////////////////////////////////////////////////////
  CConfig::CConfig() :
    m_impl(nullptr)
  {
    m_impl = new CConfigImpl();
  }

  CConfig::~CConfig()
  {
    delete m_impl;
    m_impl = nullptr;
  }

  void CConfig::OverwriteKeys(const std::vector<std::string>& key_vec_)
  {
    m_impl->OverwriteKeys(key_vec_);
  }

  void CConfig::AddFile(std::string& ini_file_)
  {
    m_impl->AddFile(ini_file_);
  }

  bool CConfig::get(const std::string& section_, const std::string& key_, bool default_)
  {
    std::string default_s("false");
    if (default_) default_s = "true";
    std::string value_s = m_impl->GetValue(section_.c_str(), key_.c_str(), default_s.c_str());
    // cause warning C4244 with VS2017
    //std::transform(value_s.begin(), value_s.end(), value_s.begin(), tolower);
    std::transform(value_s.begin(), value_s.end(), value_s.begin(),
      [](char c) {return static_cast<char>(::tolower(c)); });

    if ((value_s == "true") || (value_s == "1")) return true;
    return false;
  }

  int CConfig::get(const std::string& section_, const std::string& key_, int default_)
  {
    return static_cast<int>(m_impl->GetLongValue(section_.c_str(), key_.c_str(), static_cast<long>(default_)));
  }

  double CConfig::get(const std::string& section_, const std::string& key_, double default_)
  {
    return m_impl->GetDoubleValue(section_.c_str(), key_.c_str(), default_);
  }

  std::string CConfig::get(const std::string& section_, const std::string& key_, const char* default_)
  {
    return m_impl->GetValue(section_.c_str(), key_.c_str(), default_);
  }

  bool CfgGetBool(const std::string& section_, const std::string& key_, bool default_ /*= false*/)
  {
    return g_config()->get(section_, key_, default_);
  }

  int CfgGetInt(const std::string& section_, const std::string& key_, int default_ /*= 0*/)
  {
    return g_config()->get(section_, key_, default_);
  }

  double CfgGetDouble(const std::string& section_, const std::string& key_, double default_ /*= 0.0*/)
  {
    return g_config()->get(section_, key_, default_);
  }

  std::string CfgGetString(const std::string& section_, const std::string& key_, const char* default_ /*= ""*/)
  {
    return g_config()->get(section_, key_, default_);
  }
}
