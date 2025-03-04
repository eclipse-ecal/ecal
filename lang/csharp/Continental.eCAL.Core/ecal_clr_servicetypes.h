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

/**
 * @file ecal_clr_servicetypes.h
 * @brief Managed type definitions for eCAL services.
 *
 * This file contains managed wrappers and type definitions that mirror the native
 * eCAL service types. They can be used by the C++/CLI client and server APIs.
 */

#include "ecal_clr_datatype.h"

using namespace System;
using namespace System::Collections::Generic;

namespace Continental {
  namespace eCAL {
    namespace Core {

      /**
       * @brief Managed enumeration for service call state.
       */
      public enum class CallState {
        None = 0,      /**< Undefined */
        Executed,      /**< The service call was executed successfully */
        Timeouted,     /**< The service call has timeouted */
        Failed         /**< The service call failed */
      };

      /**
       * @brief Managed enumeration for client events.
       */
      public enum class ClientEvent {
        None = 0,          /**< Undefined */
        Connected = 1,     /**< A new server has been connected to this client */
        Disconnected = 2,  /**< A server has been disconnected from this client */
        Timeout = 3        /**< A service call has timeouted */
      };

      /**
       * @brief Managed enumeration for server events.
       */
      public enum class ServerEvent {
        None = 0,         /**< Undefined */
        Connected = 1,    /**< A new client has been connected to this server */
        Disconnected = 2  /**< A client has been disconnected from this server */
      };

      /**
       * @brief Managed wrapper for the native SServiceId structure.
       *
       * This class encapsulates the unique service entity ID and the service name.
       */
      public ref class ServiceId {
      public:
        /**
         * @brief Gets or sets the unique service entity ID.
         */
        property UInt64 EntityId;

        /**
         * @brief Gets or sets the service name.
         */
        property String^ ServiceName;

        /**
         * @brief Default constructor.
         */
        ServiceId() {}

        /**
         * @brief Parameterized constructor.
         * @param entityId Unique service entity ID.
         * @param serviceName Service name.
         */
        ServiceId(UInt64 entityId, String^ serviceName) {
          EntityId = entityId;
          ServiceName = serviceName;
        }
      };

      /**
       * @brief Managed wrapper for the native SServiceMethodInformation structure.
       *
       * This class encapsulates the service method name, request type, and response type information.
       */
      public ref class ServiceMethodInformation {
      public:
        /**
         * @brief Gets or sets the service method name.
         */
        property String^ MethodName;

        /**
         * @brief Gets or sets the request type information.
         */
        property DataTypeInformation^ RequestType;

        /**
         * @brief Gets or sets the response type information.
         */
        property DataTypeInformation^ ResponseType;

        /**
         * @brief Default constructor.
         */
        ServiceMethodInformation() {}

        /**
         * @brief Parameterized constructor.
         * @param methodName The service method name.
         * @param requestType The request type information.
         * @param responseType The response type information.
         */
        ServiceMethodInformation(String^ methodName, DataTypeInformation^ requestType, DataTypeInformation^ responseType) {
          MethodName = methodName;
          RequestType = requestType;
          ResponseType = responseType;
        }
      };

      /**
       * @brief Managed wrapper for the native SServiceResponse structure.
       *
       * This class encapsulates the service response information returned from a service call.
       */
      public ref class ServiceResponse {
      public:
        /**
         * @brief Gets or sets the call state.
         */
        property CallState CallState;

        /**
         * @brief Gets or sets the service ID of the responding server.
         */
        property ServiceId^ ServerId;

        /**
         * @brief Gets or sets the service method information.
         */
        property ServiceMethodInformation^ MethodInformation;

        /**
         * @brief Gets or sets the return state of the method call.
         */
        property int RetState;

        /**
         * @brief Gets or sets the response data as a byte array.
         */
        property array<Byte>^ Response;

        /**
         * @brief Gets or sets the error message.
         */
        property String^ ErrorMessage;
      };

      /**
       * @brief Delegate for receiving service response callbacks (client side).
       *
       * @param response The service response.
       */
      public delegate void ResponseCallback([Runtime::InteropServices::Out] ServiceResponse^ response);

      /**
       * @brief Managed wrapper for the native SClientEventCallbackData structure.
       *
       * This class encapsulates client event callback information.
       */
      public ref class ClientEventCallbackData {
      public:
        /**
         * @brief Gets or sets the client event type.
         */
        property ClientEvent Type;

        /**
         * @brief Gets or sets the event time in microseconds.
         */
        property long Time;
      };

      /**
       * @brief Delegate for client event callbacks.
       *
       * @param serviceId The service ID associated with the event.
       * @param data The client event callback data.
       */
      public delegate void ClientEventCallback(ServiceId^ serviceId, ClientEventCallbackData^ data);

      /**
       * @brief Managed wrapper for the native SServerEventCallbackData structure.
       *
       * This class encapsulates server event callback information.
       */
      public ref class ServerEventCallbackData {
      public:
        /**
         * @brief Gets or sets the server event type.
         */
        property ServerEvent Type;

        /**
         * @brief Gets or sets the event time in microseconds.
         */
        property long Time;
      };

      /**
       * @brief Delegate for server event callbacks.
       *
       * @param serviceId The service ID associated with the event.
       * @param data The server event callback data.
       */
      public delegate void ServerEventCallback(ServiceId^ serviceId, ServerEventCallbackData^ data);

      /**
       * @brief Managed type representing a list of ServiceMethodInformation.
       *
       * This can be used to pass multiple service method definitions.
       */
      public ref class ServiceMethodInformationList {
      public:
        /**
         * @brief Gets the collection of service method information.
         */
        property List<ServiceMethodInformation^>^ Methods;

        /**
         * @brief Constructor.
         */
        ServiceMethodInformationList() {
          Methods = gcnew List<ServiceMethodInformation^>();
        }
      };

    } // namespace Core
  } // namespace eCAL
} // namespace Continental
