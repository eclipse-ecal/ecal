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
 * @file   ecal_clr.h
 * @brief  eCAL C# interface
**/

#pragma once
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/dynamic_json_subscriber.h>

using namespace System;
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

        /**
         * @brief Send shutdown event to all core components.
         *
        **/
        static void ShutdownCore();

        /**
         * @brief Enable eCAL message loop back,
         *          that means subscriber will receive messages from
         *          publishers of the same process (default == false).
         *
         * @param  state_  Switch on message loop back.
        **/
        static void EnableLoopback(bool state_);
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
         * @brief Sets the log level.
         *
         * @param level_ The level.
        **/
        static void SetLogLevel(const LogLevel level_);

        /**
         * @brief Log a message.
         *
         * @param message_ The message string to log.
        **/
        static void Log(System::String^ message_);

        /**
         * @brief Set the current measured core time in s (for user implemented measuring).
        **/
        static void SetCoreTime(double time_);
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
         * @param topic_name_   Unique topic name.
         * @param topic_type_   Type name (optional.
         * @param topic_desc_   Type description (optional.
        **/
        Publisher(System::String^ topic_name_, System::String^ topic_type_, System::String^ topic_desc_);

        /**
         * @brief Destructor.
        **/
        ~Publisher();

        /**
         * @brief Creates this object.
         *
         * @param topic_name_   Unique topic name.
         * @param topic_type_   Type name (optional for type checking by monitoring app).
         *
         * @return  true if it succeeds, false if it fails.
        **/
        bool Create(System::String^ topic_name_, System::String^ topic_type_);

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
         * @brief Gets type of the connected topic.
         *
         * @return  The type name.
        **/
        System::String^ GetTypeName();

        /**
         * @brief Dump the whole class state into a string.
         *
         * @return  The dump string.
        **/
        System::String^ Dump();

      private:
        ::eCAL::CPublisher* m_pub;
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
         * @param topic_name_   Unique topic name.
         * @param topic_type_   Type name (optional for type checking by monitoring app).
         * @param topic_desc_   Descriptor (optional for dynamic reflection by monitoring app).
        **/
        Subscriber(System::String^ topic_name_, System::String^ topic_type_, System::String^ topic_desc_);

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

        /**
         * @brief delegate definition for callback functions
        **/
        delegate void ReceiverCallback(String^ str, ReceiveCallbackData^ data);

        /**
         * @brief Creates this object.
         *
         * @param topic_name_   Unique topic name.
         * @param topic_type_   Type name (optional for type checking by monitoring app).
         *
         * @return  true if it succeeds, false if it fails.
        **/
        bool Create(System::String^ topic_name_, System::String^ topic_type_);

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

        /**
         * @brief Add callback function for incoming receives.
         *
         * @param callback_  The callback function set to connect.
         *
         * @return  True if succeeded, false if not.
        **/
        bool AddReceiveCallback(ReceiverCallback^ callback_);

        /**
         * @brief Remove callback function for incoming receives.
         *
         * @param callback_  The callback function set to disconnect.
         *
         * @return  True if succeeded, false if not.
        **/
        bool RemReceiveCallback(ReceiverCallback^ callback_);

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
         * @brief Gets type of the connected topic.
         *
         * @return  The type name.
        **/
        System::String^ GetTypeName();

        /**
         * @brief Dump the whole class state into a string.
         *
         * @return  The dump sting.
        **/
        System::String^ Dump();

      private:
        ::eCAL::CSubscriber* m_sub;
        /**
         * @brief managed callbacks that will get executed on during the eCAL topic callback
        **/
        ReceiverCallback^ m_callbacks;

        /**
         * @brief private member which holds the the pointer to OnReceive, to avoid function relocation
        **/
        GCHandle m_gch;

        /**
         * @brief The callback of the subscriber, that is registered with the unmanaged code
        **/
        delegate void subCallback(const char* topic_name_, const ::eCAL::SReceiveCallbackData* data_);
        subCallback^ m_sub_callback;
        void OnReceive(const char* topic_name_, const ::eCAL::SReceiveCallbackData* data_);

        /**
         * @brief stdcall function pointer definition of eCAL::ReceiveCallbackT
        **/
        typedef void(__stdcall * stdcall_eCAL_ReceiveCallbackT)(const char*, const ::eCAL::SReceiveCallbackData*);
      };

      /**
       * @brief eCAL protobuf json subscriber class.
       *
       * The CSubscriber class is used to receive topics from matching eCAL publishers.
       *
      **/
      public ref class JSONProtobufSubscriber
      {
      public:
        /**
         * @brief Constructor.
        **/
        JSONProtobufSubscriber();

        /**
         * @brief Constructor.
         *
         * @param topic_name_   Unique topic name.
        **/
        JSONProtobufSubscriber(System::String^ topic_name_);

        /**
         * @brief Destructor.
        **/
        ~JSONProtobufSubscriber();

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

        /**
         * @brief delegate definition for callback functions
        **/
        delegate void ReceiverCallback(String^ str, ReceiveCallbackData^ data);

        /**
         * @brief Creates this object.
         *
         * @param topic_name_   Unique topic name.
        **/
        void Create(System::String^ topic_name_);

        /**
         * @brief Destroys this object.
        **/
        void Destroy();


        /**
         * @brief Add callback function for incoming receives.
         *
         * @param callback_  The callback function set to connect.
         *
         * @return  True if succeeded, false if not.
        **/
        bool AddReceiveCallback(ReceiverCallback^ callback_);

        /**
         * @brief Remove callback function for incoming receives.
         *
         * @param callback_  The callback function set to disconnect.
         *
         * @return  True if succeeded, false if not.
        **/
        bool RemReceiveCallback(ReceiverCallback^ callback_);

        /**
         * @brief Query if this object is created.
         *
         * @return  true if created, false if not.
        **/
        bool IsCreated();

      private:
        ::eCAL::protobuf::CDynamicJSONSubscriber* m_sub;
        /**
         * @brief managed callbacks that will get executed on during the eCAL topic callback
        **/
        ReceiverCallback^ m_callbacks;

        /**
         * @brief private member which holds the the pointer to OnReceive, to avoid function relocation
        **/
        GCHandle m_gch;

        /**
         * @brief The callback of the subscriber, that is registered with the unmanaged code
        **/
        delegate void subCallback(const char* topic_name_, const ::eCAL::SReceiveCallbackData* data_);
        subCallback^ m_sub_callback;
        void OnReceive(const char* topic_name_, const ::eCAL::SReceiveCallbackData* data_);

        /**
         * @brief stdcall function pointer definition of eCAL::ReceiveCallbackT
        **/
        typedef void(__stdcall * stdcall_eCAL_ReceiveCallbackT)(const char*, const ::eCAL::SReceiveCallbackData*);
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
        static System::String^ GetMonitoring();

        /**
         * @brief Get global log message protobuf string.
        **/
        static System::String^ GetLogging();
      };
    }
  }
}