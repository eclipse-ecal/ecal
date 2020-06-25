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
  namespace rec
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
        ACTION_SUPERFLOUS,
        PARAMETER_ERROR,
        ABORTED_BY_USER,
        UNSUPPORTED_ACTION,

        // Recording based errors
        NOT_STARTED,
        CURRENTLY_RECORDING,
        CURRENTLY_FLUSHING,
        CURRENTLY_UPLOADING,
        ALREADY_UPLOADED,

        MEAS_ID_NOT_FOUND,
        MEAS_IS_DELETED,

        // File based errors
        RESOURCE_UNAVAILABLE,
        DIR_NOT_EMPTY,
      };

    //////////////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////////////
    public:
      Error(ErrorCode error_code, const std::string& message) : error_code_(error_code), message_(message) {}
      Error(ErrorCode error_code) : error_code_(error_code) {}

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
        case OK:                   return "OK";                           break;
        case GENERIC_ERROR:        return "Error";                        break;

        // General Errors
        case ACTION_SUPERFLOUS:    return "Action is superflous";         break;
        case PARAMETER_ERROR:      return "Faulty or missing parameters"; break;
        case ABORTED_BY_USER:      return "Aborted by user";              break;
        case UNSUPPORTED_ACTION:   return "Unsupported action";           break;

        // Recording based errors
        case NOT_STARTED:          return "Recording not started";        break;
        case CURRENTLY_RECORDING:  return "Recording is running";         break;
        case CURRENTLY_FLUSHING:   return "The recorder is flushing";     break;
        case CURRENTLY_UPLOADING:  return "The recorder is uploading";    break;
        case ALREADY_UPLOADED:     return "Measurement already uploaded"; break;

        case MEAS_ID_NOT_FOUND:    return "Meas ID not found";            break;
        case MEAS_IS_DELETED:      return "Measurement has been deleted"; break;

        // File based errors
        case RESOURCE_UNAVAILABLE: return "Resource unavailable";         break;
        case DIR_NOT_EMPTY:        return "Directory not empty";          break;

        default:                   return "Unknown error";
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
