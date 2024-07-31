#pragma once

#include "ecal/ecal_config.h"

#include <sstream>
#include <ecal_def.h>

namespace eCAL
{
  namespace Config
  {
    std::stringstream getConfigAsYamlSS(const eCAL::Configuration& config_ = eCAL::GetConfiguration());

    bool dumpConfigToFile(const eCAL::Configuration& config_ = eCAL::GetConfiguration(), const std::string& file_path_ = ECAL_DEFAULT_CFG);
  }
}