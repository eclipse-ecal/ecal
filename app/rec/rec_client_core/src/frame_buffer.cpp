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

#include "frame_buffer.h"

#include <thread>

namespace eCAL
{
  namespace rec
  {
    // Constructor
    FrameBuffer::FrameBuffer(bool enabled, std::chrono::steady_clock::duration max_length)
      : is_enabled_(enabled)
      , max_buffer_length_(max_length)
    {}

    // Destructor
    FrameBuffer::~FrameBuffer()
    {}

    bool FrameBuffer::is_enabled() const
    {
      std::shared_lock<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);
      return is_enabled_;
    }

    void FrameBuffer::set_enabled(bool enabled)
    {
      std::unique_lock<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);

      // Clear just in case something has happend while the frame-buffer was disabled
      if (!is_enabled_)
        frame_buffer_deque_.clear();

      is_enabled_ = enabled;

      if (!is_enabled_)
        frame_buffer_deque_.clear();
    }

    std::chrono::steady_clock::duration FrameBuffer::get_max_buffer_length() const
    {
      std::shared_lock<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);
      return max_buffer_length_;
    }

    void FrameBuffer::set_max_buffer_length(std::chrono::steady_clock::duration new_length)
    {
      std::unique_lock<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);
      max_buffer_length_ = new_length;
      remove_old_frames_no_lock();
    }

    void FrameBuffer::push_back(const std::shared_ptr<Frame>& frame)
    {
      std::unique_lock<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);
      if (is_enabled_)
      {
        frame_buffer_deque_.push_back(frame);
      }
    }

    std::pair<int64_t, std::chrono::steady_clock::duration> FrameBuffer::length() const
    {
      std::shared_lock<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);

      if (!is_enabled_)
        return {0, std::chrono::steady_clock::duration(0)};

      int64_t frame_count = frame_buffer_deque_.size();
      std::chrono::steady_clock::duration buffer_length;
      if (frame_count > 0)
      {
        buffer_length = std::chrono::steady_clock::now() - frame_buffer_deque_.front()->system_receive_time_;
      }
      else
      {
        buffer_length = std::chrono::steady_clock::duration(0);
      }
      return std::make_pair(frame_count, buffer_length);
    }

    void FrameBuffer::remove_old_frames()
    {
      std::unique_lock<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);
      remove_old_frames_no_lock();
    }

    void FrameBuffer::remove_old_frames_no_lock()
    {
      auto now = std::chrono::steady_clock::now();
      
      if (frame_buffer_deque_.empty())
        return;

      if (!is_enabled_)
      {
        frame_buffer_deque_.clear();
      }
      else
      {
        auto oldest_timestamp_to_leave = now - max_buffer_length_;
        auto it = frame_buffer_deque_.begin();
        for (; it != frame_buffer_deque_.end(); it++)
        {
          if ((*it)->system_receive_time_ >= oldest_timestamp_to_leave)
            break;
        }

        frame_buffer_deque_.erase(frame_buffer_deque_.begin(), it);
      }
    }

    void FrameBuffer::clear()
    {
      std::unique_lock<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);
      frame_buffer_deque_.clear();
    }

    std::deque<std::shared_ptr<Frame>> FrameBuffer::get_as_deque() const
    {
      std::shared_lock<decltype(frame_buffer_mutex_)> frame_buffer_lock(frame_buffer_mutex_);
      if (!is_enabled_)
        return std::deque<std::shared_ptr<Frame>>();
      else
        return std::deque<std::shared_ptr<Frame>>(frame_buffer_deque_);
    }
  }
}
