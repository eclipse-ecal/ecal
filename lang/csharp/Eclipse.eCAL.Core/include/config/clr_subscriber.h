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
#include <ecal/config/subscriber.h>

#include "clr_common.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {
      namespace Config {

        /**
         * @brief Managed wrapper for the native ::eCAL::Subscriber::Layer::SHM::Configuration structure.
         */
        public ref class SubscriberLayerSHMConfiguration {
        public:
          property bool Enable;

          SubscriberLayerSHMConfiguration() {
            ::eCAL::Subscriber::Layer::SHM::Configuration native_config;
            Enable = native_config.enable;
          }

          // Native struct constructor
          SubscriberLayerSHMConfiguration(const ::eCAL::Subscriber::Layer::SHM::Configuration& native_config) {
            Enable = native_config.enable;
          }

          ::eCAL::Subscriber::Layer::SHM::Configuration ToNative() {
            ::eCAL::Subscriber::Layer::SHM::Configuration native_config;
            native_config.enable = Enable;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Subscriber::Layer::UDP::Configuration structure.
         */
        public ref class SubscriberLayerUDPConfiguration {
        public:
          property bool Enable;

          SubscriberLayerUDPConfiguration() {
            ::eCAL::Subscriber::Layer::UDP::Configuration native_config;
            Enable = native_config.enable;
          }

          // Native struct constructor
          SubscriberLayerUDPConfiguration(const ::eCAL::Subscriber::Layer::UDP::Configuration& native_config) {
            Enable = native_config.enable;
          }

          ::eCAL::Subscriber::Layer::UDP::Configuration ToNative() {
            ::eCAL::Subscriber::Layer::UDP::Configuration native_config;
            native_config.enable = Enable;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Subscriber::Layer::TCP::Configuration structure.
         */
        public ref class SubscriberLayerTCPConfiguration {
        public:
          property bool Enable;

          SubscriberLayerTCPConfiguration() {
            ::eCAL::Subscriber::Layer::TCP::Configuration native_config;
            Enable = native_config.enable;
          }

          // Native struct constructor
          SubscriberLayerTCPConfiguration(const ::eCAL::Subscriber::Layer::TCP::Configuration& native_config) {
            Enable = native_config.enable;
          }

          ::eCAL::Subscriber::Layer::TCP::Configuration ToNative() {
            ::eCAL::Subscriber::Layer::TCP::Configuration native_config;
            native_config.enable = Enable;
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Subscriber::Layer::Configuration structure.
         */
        public ref class SubscriberLayerConfiguration {
        public:
          property SubscriberLayerSHMConfiguration^ SHM;
          property SubscriberLayerUDPConfiguration^ UDP;
          property SubscriberLayerTCPConfiguration^ TCP;

          SubscriberLayerConfiguration() {
            ::eCAL::Subscriber::Layer::Configuration native_config;
            SHM = gcnew SubscriberLayerSHMConfiguration(native_config.shm);
            UDP = gcnew SubscriberLayerUDPConfiguration(native_config.udp);
            TCP = gcnew SubscriberLayerTCPConfiguration(native_config.tcp);
          }

          // Native struct constructor
          SubscriberLayerConfiguration(const ::eCAL::Subscriber::Layer::Configuration& native_config) {
            SHM = gcnew SubscriberLayerSHMConfiguration(native_config.shm);
            UDP = gcnew SubscriberLayerUDPConfiguration(native_config.udp);
            TCP = gcnew SubscriberLayerTCPConfiguration(native_config.tcp);
          }

          ::eCAL::Subscriber::Layer::Configuration ToNative() {
            ::eCAL::Subscriber::Layer::Configuration native_config;
            native_config.shm = SHM->ToNative();
            native_config.udp = UDP->ToNative();
            native_config.tcp = TCP->ToNative();
            return native_config;
          }
        };

        /**
         * @brief Managed wrapper for the native ::eCAL::Subscriber::Configuration structure.
         */
        public ref class SubscriberConfiguration {
        public:
          property SubscriberLayerConfiguration^ Layer;
          property bool DropOutOfOrderMessages;

          SubscriberConfiguration() {
            ::eCAL::Subscriber::Configuration native_config;
            Layer = gcnew SubscriberLayerConfiguration(native_config.layer);
            DropOutOfOrderMessages = native_config.drop_out_of_order_messages;
          }

          // Native struct constructor
          SubscriberConfiguration(const ::eCAL::Subscriber::Configuration& native_config) {
            Layer = gcnew SubscriberLayerConfiguration(native_config.layer);
            DropOutOfOrderMessages = native_config.drop_out_of_order_messages;
          }

          ::eCAL::Subscriber::Configuration ToNative() {
            ::eCAL::Subscriber::Configuration native_config;
            native_config.layer = Layer->ToNative();
            native_config.drop_out_of_order_messages = DropOutOfOrderMessages;
            return native_config;
          }
        };

      } // namespace Config
    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse