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

#include <deque>
#include <shared_mutex>
#include <memory>
#include <condition_variable>

#include "frame.h"

namespace eCAL
{
  namespace rec
  {
    class FrameBuffer
    {
    public:
      // Constructor
      FrameBuffer(bool enabled, std::chrono::steady_clock::duration max_length);

      // Copy
      FrameBuffer(const FrameBuffer& other)            = delete;
      FrameBuffer& operator=(const FrameBuffer& other) = delete;

      // Move
      FrameBuffer& operator=(FrameBuffer&&)      = default;
      FrameBuffer(FrameBuffer&&)                 = default;

      ~FrameBuffer();

    public:
      bool is_enabled() const;
      void set_enabled(bool enabled);

      std::chrono::steady_clock::duration get_max_buffer_length() const;
      void set_max_buffer_length(std::chrono::steady_clock::duration new_length);

      void push_back(const std::shared_ptr<Frame>& frame);
      //std::shared_ptr<Frame> pop_front();

      std::pair<int64_t, std::chrono::steady_clock::duration> length() const;

      void remove_old_frames();
      void clear();

      std::deque<std::shared_ptr<Frame>> get_as_deque() const;

    private:
      void remove_old_frames_no_lock();

    private:

      // Mutex protecting this entire class
      mutable std::shared_timed_mutex     frame_buffer_mutex_;

      // Settings
      bool                                is_enabled_;
      std::chrono::steady_clock::duration max_buffer_length_;

      // Actual frame buffer
      std::deque<std::shared_ptr<Frame>>  frame_buffer_deque_;

    };
  }
}
