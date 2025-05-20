#pragma once

#include <string>
#include <ecal/config/time.h>
#include <msclr/marshal_cppstd.h>

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Managed wrapper for the native ::eCAL::Time::Configuration structure.
         */
        public ref class TimeConfiguration {
        public:
          /**
           * @brief Gets or sets the time synchronization module for real-time.
           */
          property System::String^ TimeSyncModuleRT;

          /**
           * @brief Gets or sets the time synchronization module for replay.
           */
          property System::String^ TimeSyncModuleReplay;

          /**
           * @brief Default constructor.
           */
          TimeConfiguration() {
            // Use the default values from the native structure
            ::eCAL::Time::Configuration native_config;
            TimeSyncModuleRT = gcnew System::String(native_config.timesync_module_rt.c_str());
            TimeSyncModuleReplay = gcnew System::String(native_config.timesync_module_replay.c_str());
          }

          /**
           * @brief Native struct constructor.
           */
          TimeConfiguration(const ::eCAL::Time::Configuration& native_config) {
            TimeSyncModuleRT = gcnew System::String(native_config.timesync_module_rt.c_str());
            TimeSyncModuleReplay = gcnew System::String(native_config.timesync_module_replay.c_str());
          }

          /**
           * @brief Converts this managed object to the native structure.
           * @return Native Time::Configuration structure.
           */
          ::eCAL::Time::Configuration ToNative() {
            ::eCAL::Time::Configuration native_config;
            native_config.timesync_module_rt = msclr::interop::marshal_as<std::string>(TimeSyncModuleRT);
            native_config.timesync_module_replay = msclr::interop::marshal_as<std::string>(TimeSyncModuleReplay);
            return native_config;
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse