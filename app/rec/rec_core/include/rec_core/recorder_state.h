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

#include <chrono>
#include <list>
#include <set>
#include <string>

namespace eCAL
{
  namespace rec
  {
    struct WriterState
    {
      WriterState()
        : recording_(false)
        , flushing_(false)
        , recording_length_({ 0, std::chrono::steady_clock::duration(0) })
        , recording_queue_({ 0, std::chrono::steady_clock::duration(0) })
      {}

      bool operator== (const WriterState& other) const
      {
        return (recording_ == other.recording_)
          && (flushing_ == other.flushing_)
          && (recording_length_ == other.recording_length_)
          && (recording_queue_ == other.recording_queue_);
      }
      bool operator!= (const WriterState& other) const
      {
        return !operator==(other);
      }

      bool                                                    recording_;
      bool                                                    flushing_;
      std::pair<size_t, std::chrono::steady_clock::duration>  recording_length_;
      std::pair<size_t, std::chrono::steady_clock::duration>  recording_queue_;
    };

    struct RecorderState
    {
      RecorderState()
        : initialized_(false)
        , pre_buffer_length_({ 0, std::chrono::steady_clock::duration(0) })
      {}

      bool operator== (const RecorderState& other) const
      {
        return (initialized_ == other.initialized_)
          && (main_recorder_state_ == other.main_recorder_state_)
          && (pre_buffer_length_ == other.pre_buffer_length_)
          && (buffer_writers_ == other.buffer_writers_)
          && (subscribed_topics_ == other.subscribed_topics_);
      }

      bool operator!= (const RecorderState& other) const
      {
        return !operator==(other);
      }

      bool                                                    initialized_;
      WriterState                                             main_recorder_state_;
      std::pair<size_t, std::chrono::steady_clock::duration>  pre_buffer_length_;
      std::list<WriterState>                                  buffer_writers_;
      std::set<std::string>                                   subscribed_topics_;
    };

  }
}