/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

namespace eCAL
{
  namespace sys
  {
    class Error
    {
    //////////////////////////////////////////
    // Data model
    //////////////////////////////////////////
    public:
      enum ErrorCode
      {
        // Generic
        OK,
        GENERIC_ERROR,

        // General Errors
        NO_COMMAND,
        UNKNOWN_COMMAND,
        PARAMETER_ERROR,
        TOO_MANY_PARAMETERS,
        UNSUPPORTED_ACTION,

        // Service Errors
        COMMAND_NOT_AVAILABLE_IN_REMOTE_MODE,
        SERVICE_CALL_RETURNED_ERROR,
        REMOTE_HOST_UNAVAILABLE,

        // Task based errors
        TASK_DOES_NOT_EXIST,
        RUNNER_DOES_NOT_EXIST,
        GROUP_DOES_NOT_EXIST,
        TASK_ACTION_IS_RUNNING,

        // File based errors
        RESOURCE_UNAVAILABLE,
      };

    //////////////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////////////
    public:
      Error(ErrorCode error_code, const std::string& message) : error_code_(error_code), message_(message) {}
      Error(ErrorCode error_code) : error_code_(error_code) {}
      Error(const Error& error) : error_code_(error.error_code_), message_(error.message_) {}

      ~Error() {}

    //////////////////////////////////////////
    // Public API
    //////////////////////////////////////////
    public:
      std::string GetDescription() const
      {
        switch (error_code_)
        {
        // Generic
        case OK:                                   return "OK";                                           break;
        case GENERIC_ERROR:                        return "Error";                                        break;

        // General Errors
        case NO_COMMAND:                           return "No command";                                   break;
        case PARAMETER_ERROR:                      return "Faulty or missing parameters";                 break;
        case TOO_MANY_PARAMETERS:                  return "To many parameters";                           break;
        case UNKNOWN_COMMAND:                      return "Unknown command";                              break;
        case UNSUPPORTED_ACTION:                   return "Unsupported action";                           break;

        // Service errors
        case COMMAND_NOT_AVAILABLE_IN_REMOTE_MODE: return "Command not available in remote-control mode"; break;
        case SERVICE_CALL_RETURNED_ERROR:          return "Service call reported error";                  break;
        case REMOTE_HOST_UNAVAILABLE:              return "Remote host not available";                    break;

        // Task based errors
        case TASK_DOES_NOT_EXIST:                  return "Task does not exist";                          break;
        case RUNNER_DOES_NOT_EXIST:                return "Runner does not exist";                        break;
        case GROUP_DOES_NOT_EXIST:                 return "Group does not exist";                         break;
        case TASK_ACTION_IS_RUNNING:               return "An action is already running";                 break;

        // File based errors
        case RESOURCE_UNAVAILABLE:       return "Resource unavailable";          break;

        default:                         return "Unknown error";
        }
      }

      inline std::string ToString() const
      {
        return (message_.empty() ? GetDescription() : GetDescription() + " (" + message_ + ")");
      }

      const inline std::string& GetMessage() const
      {
        return message_;
      }

    //////////////////////////////////////////
    // Operators
    //////////////////////////////////////////
      inline operator bool() const { return error_code_ != ErrorCode::OK; }
      inline bool operator== (const Error& other) const { return error_code_ == other.error_code_; }
      inline bool operator!= (const Error& other) const { return error_code_ != other.error_code_; }

      Error& operator=(const Error& other)
      {
        error_code_ = other.error_code_;
        message_    = other.message_;
        return *this;
      }

      Error& operator=(ErrorCode error_code)
      {
        error_code_ = error_code;
        return *this;
      }

    //////////////////////////////////////////
    // Member Variables
    //////////////////////////////////////////
    private:
      ErrorCode   error_code_;
      std::string message_;
    };
  }
}
