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
 * @file   log.cpp
 * @brief  eCAL logging c interface
**/

#include <ecal/ecal.h>
#include <ecal_c/log.h>

#include "common.h"

#include <map>
#include <cassert>
#include <numeric>

namespace
{
  size_t ExtSize_Logging_SLogMessage(const eCAL::Logging::SLogMessage log_message_)
  {
    return ExtSize_String(log_message_.content) +
      ExtSize_String(log_message_.host_name) +
      ExtSize_String(log_message_.process_name) +
      ExtSize_String(log_message_.unit_name);
  }

  size_t ExtSize_Logging_SLogging(const eCAL::Logging::SLogging& logging_)
  {
    return aligned_size(sizeof(struct eCAL_Logging_SLogMessage) * logging_.log_messages.size())
      + std::accumulate(logging_.log_messages.begin(), logging_.log_messages.end(), size_t{ 0 },
                            [](auto size_, const auto& log_message_){
                               return size_ + ExtSize_Logging_SLogMessage(log_message_);
                            });
  }

  void Assign_Logging_SLogMessage(struct eCAL_Logging_SLogMessage* log_message_c_, const eCAL::Logging::SLogMessage& log_message_, char** offset_)
  {
    static const std::map<eCAL::Logging::eLogLevel, eCAL_Logging_eLogLevel> log_level_map
    {
        {eCAL::Logging::log_level_none, eCAL_Logging_log_level_none},
        {eCAL::Logging::log_level_all, eCAL_Logging_log_level_all},
        {eCAL::Logging::log_level_info, eCAL_Logging_log_level_info},
        {eCAL::Logging::log_level_warning, eCAL_Logging_log_level_warning},
        {eCAL::Logging::log_level_error, eCAL_Logging_log_level_error},
        {eCAL::Logging::log_level_fatal, eCAL_Logging_log_level_fatal},
        {eCAL::Logging::log_level_debug1, eCAL_Logging_log_level_debug1},
        {eCAL::Logging::log_level_debug2, eCAL_Logging_log_level_debug2},
        {eCAL::Logging::log_level_debug3, eCAL_Logging_log_level_debug3},
        {eCAL::Logging::log_level_debug4, eCAL_Logging_log_level_debug4}
    };

    log_message_c_->content = Convert_String(log_message_.content, offset_);
    log_message_c_->host_name = Convert_String(log_message_.host_name, offset_);
    log_message_c_->level = log_level_map.at(log_message_.level);
    log_message_c_->process_id = log_message_.process_id;
    log_message_c_->process_name = Convert_String(log_message_.process_name, offset_);
    log_message_c_->time = log_message_.time;
    log_message_c_->unit_name = Convert_String(log_message_.unit_name, offset_);
  }

  void Assign_Logging_SLogging(struct eCAL_Logging_SLogging* logging_c_, const eCAL::Logging::SLogging& logging_, char** offset_)
  {
    logging_c_->log_messages = reinterpret_cast<struct eCAL_Logging_SLogMessage*>(*offset_);
    logging_c_->log_messages_length = logging_.log_messages.size();

    *offset_ += aligned_size(sizeof(struct eCAL_Logging_SLogMessage) * logging_.log_messages.size());
    size_t i{ 0 };
    for (const auto& log_message : logging_.log_messages)
    {
      Assign_Logging_SLogMessage(&(logging_c_->log_messages[i++]), log_message, offset_);
    }
  }
}

extern "C"
{
  ECALC_API void eCAL_Logging_Log(enum eCAL_Logging_eLogLevel level_, const char* message_)
  {
    assert(message_ != NULL);
    static const std::map<eCAL_Logging_eLogLevel, eCAL::Logging::eLogLevel> log_level_map
    {
        {eCAL_Logging_log_level_none, eCAL::Logging::log_level_none},
        {eCAL_Logging_log_level_all, eCAL::Logging::log_level_all},
        {eCAL_Logging_log_level_info, eCAL::Logging::log_level_info},
        {eCAL_Logging_log_level_warning, eCAL::Logging::log_level_warning},
        {eCAL_Logging_log_level_error, eCAL::Logging::log_level_error},
        {eCAL_Logging_log_level_fatal, eCAL::Logging::log_level_fatal},
        {eCAL_Logging_log_level_debug1, eCAL::Logging::log_level_debug1},
        {eCAL_Logging_log_level_debug2, eCAL::Logging::log_level_debug2},
        {eCAL_Logging_log_level_debug3, eCAL::Logging::log_level_debug3},
        {eCAL_Logging_log_level_debug4, eCAL::Logging::log_level_debug4}
    };
    eCAL::Logging::Log(log_level_map.at(level_), message_);
  }

  ECALC_API int eCAL_Logging_GetLoggingBuffer(void** logging_buffer_, size_t* logging_buffer_length_)
  {
    assert(logging_buffer_ != NULL && logging_buffer_length_ != NULL);
    assert(*logging_buffer_ == NULL && *logging_buffer_length_ == 0);

    std::string buffer;
    if (eCAL::Logging::GetLogging(buffer))
    {
      *logging_buffer_ = std::malloc(buffer.size());
      if (*logging_buffer_ != NULL)
      {
        std::memcpy(*logging_buffer_, buffer.data(), buffer.size());
        *logging_buffer_length_ = buffer.size();
        
      }
    }
    return !static_cast<int>(*logging_buffer_ != NULL);
  }

  ECALC_API int eCAL_Logging_GetLogging(struct eCAL_Logging_SLogging** logging_)
  {
    assert(logging_ != NULL);
    assert(*logging_ == NULL);

    eCAL::Logging::SLogging logging;
    if (eCAL::Logging::GetLogging(logging))
    {
      const auto base_size = aligned_size(sizeof(struct eCAL_Logging_SLogging));
      const auto extented_size = ExtSize_Logging_SLogging(logging);
      *logging_ = reinterpret_cast<struct eCAL_Logging_SLogging*>(std::malloc(base_size + extented_size));
      if (*logging_ != NULL)
      {
        auto* offset = reinterpret_cast<char*>(*logging_) + base_size;
        Assign_Logging_SLogging(*logging_, logging, &offset);
      }
    }

    return !static_cast<int>(*logging_ != NULL);
  }
}
