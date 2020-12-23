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

/**
 * eCALSys configuration file
**/

#include "ecalsys_settings.h"

#include <memory>

#include "SimpleIni.h"

#include <ecal_def_ini.h>
#include <ecal_utils/filesystem.h>

EcalsysSettings::EcalsysSettings(void)
  : is_init_(false)
  , file_name_("")
  , apps_filter_denied_("")
{}

EcalsysSettings::~EcalsysSettings(void)
{
  Destroy();
}

void EcalsysSettings::Create(const std::string& path)
{
  if (is_init_) return;

  // load ini file
  LoadIniFile(path);

  is_init_ = true;
}

void EcalsysSettings::Destroy()
{
  if (!is_init_) return;

  is_init_ = false;
}

void EcalsysSettings::LoadIniFile(const std::string& path)
{
  // store config file parameter
  if (!path.empty())
  {
    file_name_ = path;
  }

  // load settings from file
  auto file_info = EcalUtils::Filesystem::FileStatus(file_name_);
  if (file_info.IsOk() && (file_info.GetType() == EcalUtils::Filesystem::Type::RegularFile))
  {
    // apply file name to manager
    std::shared_ptr<CSimpleIniA> iniConf(new CSimpleIniA(true, true, true));
    SI_Error rc = iniConf->LoadFile(file_name_.c_str());
    if (rc < 0) return;

    // Filter list for import from cloud
    apps_filter_denied_   = iniConf->GetValue(SYS_SECTION_S, SYS_FILTER_EXCL_S, "");
  }
}
