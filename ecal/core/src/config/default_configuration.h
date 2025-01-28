#pragma once

#include "ecal/config.h"

#include <sstream>
#include <ecal_def.h>

namespace eCAL
{
  namespace Config
  {
    std::stringstream getConfigAsYamlSS(const eCAL::Configuration& config_ = eCAL::GetConfiguration());

    std::stringstream getTimeConfigAsYamlSS();
  }
}