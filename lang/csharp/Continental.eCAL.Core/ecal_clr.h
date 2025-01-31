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

/**
 * @file   ecal_clr.h
 * @brief  eCAL C# interface
**/

#pragma once
#include <ecal/ecal.h>
#include <ecal/v5/ecal_client.h>
#include <ecal/v5/ecal_server.h>
#include <ecal/v5/ecal_publisher.h>
#include <ecal/v5/ecal_subscriber.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

namespace Continental
{
  namespace eCAL
  {
    namespace Core
    {
      /**
       * @brief eCAL utility class.
      **/
      public ref class Util
      {
      public:
        /**
        * @brief Initialize eCAL API.
        *
        * @param task_name_ The instance task name.
        **/
        static void Initialize(System::String^ task_name_);

        /**
        * @brief Finalize eCAL API.
        **/
        static void Terminate();

        /**
         * @brief Retrieve eCAL release version string.
         *
         * @return  The version string.
        **/
        static System::String^ GetVersion();

        /**
         * @brief Retrieve eCAL release date string.
         *
         * @return  The date string.
        **/
        static System::String^ GetDate();

        /**
         * @brief Return the eCAL process state.
         *
         * @return  True if eCAL is in proper state.
        **/
        static bool Ok();

        /**
         * @brief Send shutdown event to all user processes.
        **/
        static void ShutdownProcesses();
      };


      /**
       * @brief Values that represent different log level to filter on monitoring.
      **/
      enum LogLevel
      {
        info = 1,
        warning,
        error,
        fatal,
      };

      /**
       * @brief eCAL logging class.
      **/
      public ref class Logger
      {
      public:
        /**
         * @brief Log a message.
         *
         * @param message_ The message string to log.
        **/
        static void Log(System::String^ message_);
      };


      /**
       * @brief eCAL publisher class.
       *
       * The CPublisher class is used to send topics to matching eCAL subscribers. The topic is created automatically by the constructor
       * or by the Create member function.
       *
      **/
      public ref class Publisher
      {
      public:
        /**
         * @brief Constructor.
        **/
        Publisher();

        /**
         * @brief Constructor.
         *
         * @param topic_name_     Unique topic name.
         * @param topic_type_     Type name (optional).
         * @param topic_encoding_ Type encoding (optional).
         * @param topic_desc_     Type description (optional).
        **/
        Publisher(System::String^ topic_name_, System::String^ topic_type_, System::String^ topic_encoding_, System::String^ topic_desc_);

        /**
         * @brief Constructor.
         *
         * @param topic_name_     Unique topic name.
         * @param topic_type_     Type name (optional.
         * @param topic_encoding_ Type encoding (optional).
         * @param topic_desc_     Type description (optional.
        **/
        Publisher(System::String^ topic_name_, System::String^ topic_type_, System::String^ topic_encoding_, array<Byte>^ topic_desc_);

        /**
         * @brief Destructor.
        **/
        ~Publisher();

        /**
         * @brief Creates this object.
         *
         * @param topic_name_     Unique topic name.
         * @param topic_type_     Type name (optional for type checking by monitoring app).
         * @param topic_encoding_ Type encoding (optional).
         *
         * @return  true if it succeeds, false if it fails.
        **/
        bool Create(System::String^ topic_name_, System::String^ topic_encoding_, System::String^ topic_type_);

        /**
         * @brief Destroys this object.
         *
         * @return  true if it succeeds, false if it fails.
        **/
        bool Destroy();

        /**
         * @brief Send a message to all subscribers.
         *
         * @param s_      String that contains content to send.
         * @param time_   Send time (-1 = use eCAL system time in us, default = -1).
         *
         * @return  number of bytes sent.
        **/
        size_t Send(System::String^ s_, const long long time_);

        /**
         * @brief Send a message to all subscribers.
         *
         * @param buffer_ byte[] that contains the message content.
         * @param time_   Send time (-1 = use eCAL system time in us, default = -1).
         *
         * @return  number of bytes sent.
        **/
        size_t Send(array<Byte>^ buffer_, long long time_);

        /**
         * @brief Query if this object is created.
         *
         * @return  true if created, false if not.
        **/
        bool IsCreated();

        /**
         * @brief Gets name of the connected topic.
         *
         * @return  The topic name.
        **/
        System::String^ GetTopicName();

        /**
         * @brief Dump the whole class state into a string.
         *
         * @return  The dump string.
        **/
        System::String^ Dump();

      private:
        ::eCAL::v5::CPublisher* m_pub;
      };


      /**
       * @brief eCAL subscriber class.
       *
       * The CSubscriber class is used to receive topics from matching eCAL publishers.
       *
      **/
      public ref class Subscriber
      {
      public:
        /**
         * @brief Constructor.
        **/
        Subscriber();

        /**
         * @brief Constructor.
         *
         * @param topic_name_     Unique topic name.
         * @param topic_type_     Type name (optional).
         * @param topic_encoding_ Type encoding (optional).
         * @param topic_desc_     Type description (optional).
        **/
        Subscriber(System::String^ topic_name_, System::String^ topic_type_, System::String^ topic_encoding_, System::String^ topic_desc_);

        /**
         * @brief Constructor.
         *
         * @param topic_name_     Unique topic name.
         * @param topic_type_     Type name (optional.
         * @param topic_encoding_ Type encoding (optional).
         * @param topic_desc_     Type description (optional.
        **/
        Subscriber(System::String^ topic_name_, System::String^ topic_type_, System::String^ topic_encoding_, array<Byte>^ topic_desc_);

        /**
         * @brief Destructor.
        **/
        ~Subscriber();

        /**
         * @brief structure which contains the data for callback functions
        **/
        ref struct ReceiveCallbackData
        {
          System::String^ data;  /*!< Message payload     */
          long long id;          /*!< Message id          */
          long long time;        /*!< Message time stamp  */
          long long clock;       /*!< Message write clock */
        };

        ref struct ReceiveCallbackDataUnsafe
        {
          void* data;           /*!< Message payload     */
          unsigned long long size;   /*!< Message payload length*/
          long long id;         /*!< Message id          */
          long long time;       /*!< Message time stamp  */
          long long clock;      /*!< Message write clock */
        };
        /**
         * @brief delegate definition for callback functions
        **/
        delegate void ReceiverCallback(String^ str, ReceiveCallbackData^ data);
        delegate void ReceiverCallbackUnsafe(String^ str, ReceiveCallbackDataUnsafe^ data);

        /**
         * @brief Creates this object.
         *
         * @param topic_name_     Unique topic name.
         * @param topic_type_     Type name (optional for type checking by monitoring app).
         * @param topic_encoding_ Type encoding (optional).
         *
         * @return  true if it succeeds, false if it fails.
        **/
        bool Create(System::String^ topic_name_, System::String^ topic_encoding_, System::String^ topic_type_);

        /**
         * @brief Destroys this object.
         *
         * @return  true if it succeeds, false if it fails.
        **/
        bool Destroy();

        /**
         * @brief Receive a message from the publisher.
         *
         * @param rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
         *
         * @return  ReceiveCallbackData or null (if timed out)
        **/
        ReceiveCallbackData^ Receive(const int rcv_timeout_);

        ReceiveCallbackDataUnsafe^ ReceiveUnsafe(const int rcv_timeout_);
        /**
         * @brief Add callback function for incoming receives.
         *
         * @param callback_  The callback function set to connect.
         *
         * @return  True if succeeded, false if not.
        **/
        bool AddReceiveCallback(ReceiverCallback^ callback_);

        bool AddReceiveCallback(ReceiverCallbackUnsafe^ callback_);
        /**
         * @brief Remove callback function for incoming receives.
         *
         * @param callback_  The callback function set to disconnect.
         *
         * @return  True if succeeded, false if not.
        **/
        bool RemReceiveCallback(ReceiverCallback^ callback_);

        bool RemReceiveCallback(ReceiverCallbackUnsafe^ callback_);
        /**
         * @brief Query if this object is created.
         *
         * @return  true if created, false if not.
        **/
        bool IsCreated();

        /**
         * @brief Gets name of the connected topic.
         *
         * @return  The topic name.
        **/
        System::String^ GetTopicName();

        /**
         * @brief Dump the whole class state into a string.
         *
         * @return  The dump sting.
        **/
        System::String^ Dump();

      private:
        ::eCAL::v5::CSubscriber* m_sub;
        /**
         * @brief managed callbacks that will get executed on during the eCAL topic callback
        **/
        ReceiverCallback^ m_callbacks;

        ReceiverCallbackUnsafe^ m_callbacks_unsafe;
        /**
         * @brief private member which holds the the pointer to OnReceive, to avoid function relocation
        **/
        GCHandle m_gch;

        GCHandle m_gch_unsafe;
        /**
         * @brief The callback of the subscriber, that is registered with the unmanaged code
        **/
        delegate void subCallback(const char* topic_name_, const ::eCAL::v5::SReceiveCallbackData* data_);
        subCallback^ m_sub_callback;
        void OnReceive(const char* topic_name_, const ::eCAL::v5::SReceiveCallbackData* data_);
        void OnReceiveUnsafe(const char* topic_name_, const ::eCAL::v5::SReceiveCallbackData* data_);
        /**
         * @brief stdcall function pointer definition of eCAL::ReceiveCallbackT
        **/
        typedef void(__stdcall * stdcall_eCAL_ReceiveCallbackT)(const char*, const ::eCAL::v5::SReceiveCallbackData*);
      };


      /**
       * @brief eCAL server class.
       *
       * The CServiceServer class is used to answer calls from matching eCAL clients.
       *
      **/
      public ref class ServiceServer
      {
      public:
          /**
           * @brief Constructor.
          **/
          ServiceServer();

          /**
           * @brief Constructor.
           *
           * @param topic_name_   Unique server name.
          **/
          ServiceServer(System::String^ server_name_);

          /**
           * @brief Destructor.
          **/
          ~ServiceServer();

        /**
         * @brief delegate definition for callback functions
        **/
        delegate array<Byte>^ MethodCallback(String^ methodName, String^ reqType, String^ responseType, array<Byte>^ request);

        /**
         * @brief Destroys this object.
         *
         * @return  true if it succeeds, false if it fails.
        **/
        bool Destroy();

        /**
         * @brief Add callback function for incoming calls.
         *
         * @param callback_  The callback function set to connect.
         *
         * @return  True if succeeded, false if not.
        **/
        bool AddMethodCallback(String^ methodName, String^ reqType, String^ responseType, MethodCallback^ callback_);

        /**
         * @brief Remove callback function for incoming calls.
         *
         * @param callback_  The callback function set to disconnect.
         *
         * @return  True if succeeded, false if not.
        **/
        bool RemMethodCallback(String^ methodName, MethodCallback^ callback_);

      private:
        ::eCAL::v5::CServiceServer* m_serv;
        /**
         * @brief managed callbacks that will get executed during the eCAL method callback
        **/
        MethodCallback^ m_callbacks;

        /**
         * @brief private member which holds the pointer to OnMethodCall, to avoid function relocation
        **/
        GCHandle m_gch;

        /**
         * @brief The callback of the subscriber, that is registered with the unmanaged code
        **/
        delegate int servCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_);
        servCallback^ m_sub_callback;
        int OnMethodCall(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_);

        /**
         * @brief stdcall function pointer definition of eCAL::ReceiveCallbackT
        **/
        typedef int(__stdcall * stdcall_eCAL_MethodCallbackT)(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_);
      };


      /**
       * @brief eCAL service client class.
       *
       * The CServiceClient class is used to call a matching eCAL server.
       *
      **/
      public ref class ServiceClient
      {
      public:
          /**
           * @brief Constructor.
          **/
          ServiceClient();

          /**
           * @brief Constructor.
           *
           * @param service_name_   Unique service name.
          **/
          ServiceClient(System::String^ service_name_);

          /**
           * @brief Destructor.
          **/
          ~ServiceClient();

          enum class CallState
          {
            None = 0,    //!< undefined
            Executed,    //!< executed (successfully)
            Failed       //!< failed
          };
          /**
           * @brief structure which contains the data for callback functions
          **/
          ref struct ServiceClientCallbackData
          {
              String^      host_name;      //!< service host name
              String^      service_name;   //!< name of the service
              String^      service_id;     //!< id of the service
              String^      method_name;    //!< name of the service method
              String^      error_msg;      //!< human readable error message
              int          ret_state;      //!< return state of the called service method
              CallState    call_state;     //!< call state (see eCallState)
              array<Byte>^ response;       //!< service response
          };

          /**
           * @brief Call a server.
           *
           * @param method_name_
           * @param request
           * @param rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
           *
           * @return  List<ServiceClientCallbackData> or null (if timed out)
          **/
          List<ServiceClientCallbackData^>^ Call(System::String^ method_name_, array<Byte>^ request, const int rcv_timeout_);

      private:
          ::eCAL::v5::CServiceClient* m_client;
      };


      /**
       * @brief eCAL monitoring class.
      **/
      public ref class Monitoring
      {
      public:
        /**
         * @brief Initialize eCAL Monitoring API.
        **/
        static void Initialize();

        /**
         * @brief Finalize eCAL Monitoring API.
        **/
        static void Terminate();

        /**
         * @brief Get host, process and topic protobuf string.
        **/
        static String^ GetMonitoring();

        /**
         * @brief Get global log message protobuf string.
        **/
        static String^ GetLogging();

        /**
         * @brief Get host, process and topic protobuf message bytes.
        **/
        static array<Byte>^ GetMonitoringBytes();

        /**
         * @brief Get global log message protobuf message bytes.
        **/
        static array<Byte>^ GetLoggingBytes();

        /*
        * @brief Get eCAL time
        */
        static DateTime GetTime();
      };
    }
  }
}