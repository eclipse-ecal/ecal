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

#include "ecal/types/logging.h"
#include "clr_log_level.h"
#include "../clr_common.h"

namespace Eclipse {
  namespace eCAL {
    namespace Core {
    // Managed wrapper for the native eCAL::Logging::SLogMessage struct
    public ref class SLogMessage
    {
      public:
        property System::Int64 Time;
        property System::String^ HostName;
        property System::Int32 ProcessId;
        property System::String^ ProcessName;
        property System::String^ UnitName;
        property ::Eclipse::eCAL::Core::eLoggingLogLevel Level;
        property System::String^ Content;

        SLogMessage()
        {
          ::eCAL::Logging::SLogMessage native_msg;
          Time = native_msg.time;
          HostName = ::Internal::StlStringToString(native_msg.host_name);
          ProcessId = native_msg.process_id;
          ProcessName = ::Internal::StlStringToString(native_msg.process_name);
          UnitName = ::Internal::StlStringToString(native_msg.unit_name);
          Level = ::Eclipse::eCAL::Core::eLoggingLogLevelHelper::FromNative(native_msg.level);
          Content = ::Internal::StlStringToString(native_msg.content);
        }

        // Native to managed conversion
        static SLogMessage^ FromNative(const ::eCAL::Logging::SLogMessage& native)
        {
          auto msg = gcnew SLogMessage();
          msg->Time = native.time;
          msg->HostName = ::Internal::StlStringToString(native.host_name);
          msg->ProcessId = native.process_id;
          msg->ProcessName = ::Internal::StlStringToString(native.process_name);
          msg->UnitName = ::Internal::StlStringToString(native.unit_name);
          msg->Level = ::Eclipse::eCAL::Core::eLoggingLogLevelHelper::FromNative(native.level);
          msg->Content = ::Internal::StlStringToString(native.content);
          return msg;
        }

        // Managed to native conversion
        ::eCAL::Logging::SLogMessage ToNative()
        {
          ::eCAL::Logging::SLogMessage native;
          native.time = Time;
          native.host_name = ::Internal::StringToStlString(HostName);
          native.process_id = ProcessId;
          native.process_name = ::Internal::StringToStlString(ProcessName);
          native.unit_name = ::Internal::StringToStlString(UnitName);
          native.level = ::Eclipse::eCAL::Core::eLoggingLogLevelHelper::ToNative(Level);
          native.content = ::Internal::StringToStlString(Content);
          return native;
        }
    };

    public ref class SLogging
    {
    public:
      property System::Collections::Generic::List<SLogMessage^>^ LogMessages;

      SLogging()
      {
        LogMessages = gcnew System::Collections::Generic::List<SLogMessage^>();
      }

      // Native to managed conversion
      static SLogging^ FromNative(const ::eCAL::Logging::SLogging& native)
      {
        auto logging = gcnew SLogging();
        for (const auto& msg : native.log_messages)
        {
          logging->LogMessages->Add(SLogMessage::FromNative(msg));
        }
        return logging;
      }

      // Managed to native conversion
      ::eCAL::Logging::SLogging ToNative()
      {
        ::eCAL::Logging::SLogging native;
        for each (SLogMessage^ msg in LogMessages)
        {
          native.log_messages.push_back(msg->ToNative());
        }
        return native;
      }
    };
    }
  }
}