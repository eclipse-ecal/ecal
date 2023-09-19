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

#pragma once

#include <string>

namespace eCAL
{
  namespace service
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

        // Client Calls
        CONNECTION_CLOSED,
        UNKNOWN_SERVER,
        PROTOCOL_ERROR,
        STOPPED_BY_USER,
      };

    //////////////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////////////
    public:
      Error(ErrorCode error_code, const std::string& message) : error_code_(error_code), message_(message) {}
      Error(ErrorCode error_code) : error_code_(error_code) {}

    //////////////////////////////////////////
    // Public API
    //////////////////////////////////////////
    public:
      inline std::string GetDescription() const
      {
        switch (error_code_)
        {
        // Generic
        case OK:                                    return "OK";                                            break;
        case GENERIC_ERROR:                         return "Error";                                         break;

        // Client Calls
        case CONNECTION_CLOSED:                     return "Connection closed";                             break;  
        case UNKNOWN_SERVER:                        return "Unknown server";                                break;
        case PROTOCOL_ERROR:                        return "Protocol error";                                break;
        case STOPPED_BY_USER:                       return "Stopped by user";                               break;

        default:                                    return "Unknown error";
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
      inline bool operator== (const Error& other)    const { return error_code_ == other.error_code_; }
      inline bool operator== (const ErrorCode other) const { return error_code_ == other; }
      inline bool operator!= (const Error& other)    const { return error_code_ != other.error_code_; }
      inline bool operator!= (const ErrorCode other) const { return error_code_ != other; }

      inline Error& operator=(ErrorCode error_code)
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

  } // namespace service
} // namespace eCAL
