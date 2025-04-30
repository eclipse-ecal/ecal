#pragma once

#include <ecal/init.h>
#include <msclr/marshal_cppstd.h>
#include "config/clr_configuration.h"

using namespace System;

namespace Eclipse {
  namespace eCAL {
    namespace Core {

      /**
       * @brief Managed wrapper for the eCAL initialization functionality.
       */
      public ref class InitWrapper {
      public:
        /**
         * @brief Managed enumeration for eCAL initialization flags.
         */
        [Flags]
        enum class InitFlags : unsigned int {
          None       = ::eCAL::Init::None,
          Publisher  = ::eCAL::Init::Publisher,
          Subscriber = ::eCAL::Init::Subscriber,
          Service    = ::eCAL::Init::Service,
          Monitoring = ::eCAL::Init::Monitoring,
          Logging    = ::eCAL::Init::Logging,
          TimeSync   = ::eCAL::Init::TimeSync,
          Default    = ::eCAL::Init::Default,
          All        = ::eCAL::Init::All
        };

        /**
         * @brief Gets the init eCAL configuration.
         * @return Managed wrapper for the init configuration.
         */
        static Config::Configuration^ InitConfiguration();
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse