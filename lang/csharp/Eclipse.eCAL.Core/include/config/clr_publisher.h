/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#pragma once

#include <string>
#include <vector>
#include <ecal/config/publisher.h>

using namespace System::Collections::Generic;

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Managed wrapper for the native ::eCAL::Publisher::Layer::SHM::Configuration structure.
         */
        public ref class PublisherLayerSHMConfiguration {
        public:
          property bool Enable;
          property bool ZeroCopyMode;
          property unsigned int AcknowledgeTimeoutMs;
          property unsigned int MemfileBufferCount;
          property unsigned int MemfileMinSizeBytes;
          property unsigned int MemfileReservePercent;

          PublisherLayerSHMConfiguration() {
            ::eCAL::Publisher::Layer::SHM::Configuration native_config;
            Enable = native_config.enable;
            ZeroCopyMode = native_config.zero_copy_mode;
            AcknowledgeTimeoutMs = native_config.acknowledge_timeout_ms;
            MemfileBufferCount = native_config.memfile_buffer_count;
            MemfileMinSizeBytes = native_config.memfile_min_size_bytes;
            MemfileReservePercent = native_config.memfile_reserve_percent;
          }

          // Native struct constructor
          PublisherLayerSHMConfiguration(const ::eCAL::Publisher::Layer::SHM::Configuration& native_config) {
            Enable = native_config.enable;
            ZeroCopyMode = native_config.zero_copy_mode;
            AcknowledgeTimeoutMs = native_config.acknowledge_timeout_ms;
            MemfileBufferCount = native_config.memfile_buffer_count;
            MemfileMinSizeBytes = native_config.memfile_min_size_bytes;
            MemfileReservePercent = native_config.memfile_reserve_percent;
          }

          ::eCAL::Publisher::Layer::SHM::Configuration ToNative() {
            ::eCAL::Publisher::Layer::SHM::Configuration native_config;
            native_config.enable = Enable;
            native_config.zero_copy_mode = ZeroCopyMode;
            native_config.acknowledge_timeout_ms = AcknowledgeTimeoutMs;
            native_config.memfile_buffer_count = MemfileBufferCount;
            native_config.memfile_min_size_bytes = MemfileMinSizeBytes;
            native_config.memfile_reserve_percent = MemfileReservePercent;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Publisher::Layer::UDP::Configuration structure.
         */
        public ref class PublisherLayerUDPConfiguration {
        public:
          property bool Enable;

          PublisherLayerUDPConfiguration() {
            ::eCAL::Publisher::Layer::UDP::Configuration native_config;
            Enable = native_config.enable;
          }

          // Native struct constructor
          PublisherLayerUDPConfiguration(const ::eCAL::Publisher::Layer::UDP::Configuration& native_config) {
            Enable = native_config.enable;
          }

          ::eCAL::Publisher::Layer::UDP::Configuration ToNative() {
            ::eCAL::Publisher::Layer::UDP::Configuration native_config;
            native_config.enable = Enable;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Publisher::Layer::TCP::Configuration structure.
         */
        public ref class PublisherLayerTCPConfiguration {
        public:
          property bool Enable;

          PublisherLayerTCPConfiguration() {
            ::eCAL::Publisher::Layer::TCP::Configuration native_config;
            Enable = native_config.enable;
          }

          // Native struct constructor
          PublisherLayerTCPConfiguration(const ::eCAL::Publisher::Layer::TCP::Configuration& native_config) {
            Enable = native_config.enable;
          }

          ::eCAL::Publisher::Layer::TCP::Configuration ToNative() {
            ::eCAL::Publisher::Layer::TCP::Configuration native_config;
            native_config.enable = Enable;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Publisher::Layer::Configuration structure.
         */
        public ref class PublisherLayerConfiguration {
        public:
          property PublisherLayerSHMConfiguration^ SHM;
          property PublisherLayerUDPConfiguration^ UDP;
          property PublisherLayerTCPConfiguration^ TCP;

          PublisherLayerConfiguration() {
            ::eCAL::Publisher::Layer::Configuration native_config;
            SHM = gcnew PublisherLayerSHMConfiguration(native_config.shm);
            UDP = gcnew PublisherLayerUDPConfiguration(native_config.udp);
            TCP = gcnew PublisherLayerTCPConfiguration(native_config.tcp);
          }

          // Native struct constructor
          PublisherLayerConfiguration(const ::eCAL::Publisher::Layer::Configuration& native_config) {
            SHM = gcnew PublisherLayerSHMConfiguration(native_config.shm);
            UDP = gcnew PublisherLayerUDPConfiguration(native_config.udp);
            TCP = gcnew PublisherLayerTCPConfiguration(native_config.tcp);
          }

          ::eCAL::Publisher::Layer::Configuration ToNative() {
            ::eCAL::Publisher::Layer::Configuration native_config;
            native_config.shm = SHM->ToNative();
            native_config.udp = UDP->ToNative();
            native_config.tcp = TCP->ToNative();
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Publisher::Configuration structure.
         */
        public ref class PublisherConfiguration {
        public:
          property PublisherLayerConfiguration^ Layer;
          property List<int>^ LayerPriorityLocal;
          property List<int>^ LayerPriorityRemote;

          PublisherConfiguration() {
            ::eCAL::Publisher::Configuration native_config;
            Layer = gcnew PublisherLayerConfiguration(native_config.layer);

            LayerPriorityLocal = gcnew List<int>();
            for (auto layer : native_config.layer_priority_local) {
              LayerPriorityLocal->Add(static_cast<int>(layer));
            }

            LayerPriorityRemote = gcnew List<int>();
            for (auto layer : native_config.layer_priority_remote) {
              LayerPriorityRemote->Add(static_cast<int>(layer));
            }
          }

          // Native struct constructor
          PublisherConfiguration(const ::eCAL::Publisher::Configuration& native_config) {
            Layer = gcnew PublisherLayerConfiguration(native_config.layer);

            LayerPriorityLocal = gcnew List<int>();
            for (auto layer : native_config.layer_priority_local) {
              LayerPriorityLocal->Add(static_cast<int>(layer));
            }

            LayerPriorityRemote = gcnew List<int>();
            for (auto layer : native_config.layer_priority_remote) {
              LayerPriorityRemote->Add(static_cast<int>(layer));
            }
          }

          ::eCAL::Publisher::Configuration ToNative() {
            ::eCAL::Publisher::Configuration native_config;
            native_config.layer = Layer->ToNative();

            native_config.layer_priority_local.clear();
            for each (int layer in LayerPriorityLocal) {
              native_config.layer_priority_local.push_back(static_cast<::eCAL::TransportLayer::eType>(layer));
            }

            native_config.layer_priority_remote.clear();
            for each (int layer in LayerPriorityRemote) {
              native_config.layer_priority_remote.push_back(static_cast<::eCAL::TransportLayer::eType>(layer));
            }

            return native_config;
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse