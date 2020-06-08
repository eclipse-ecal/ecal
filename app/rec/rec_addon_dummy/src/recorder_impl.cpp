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

#include "recorder_impl.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <iostream>
#endif
#include <sstream>

std::unique_ptr<eCAL::rec::addon::RecorderImplBase> eCAL::rec::addon::recorder_impl = std::make_unique<RecorderImpl>();

struct Frame : eCAL::rec::addon::BaseFrame
{
  std::string payload;
};

bool RecorderImpl::Initialize()
{
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  if (is_initialized_) return false;
  is_initialized_ = true;
  std::thread thread([this]()
  {
    while (is_initialized_)
    {
      auto frame = std::make_shared<Frame>();
      static int c = 0;
      frame->payload = std::to_string(++c);
      std::stringstream s;
      if (RecordFrame(frame))
      {
        s << "Frame " << c << " recorded" << std::endl;
        //OutputDebugStringA(s.str().c_str());
        //std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
      else
      {
        s << "Frame " << c << " dropped " << std::endl;
#ifdef WIN32
        OutputDebugStringA(s.str().c_str());
#else
        std::cerr << s.str() << std::endl;
#endif
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    }
  });
  std::swap(thread_, thread);

  SetLastStatus("Dummy receiver is connected.");
  return true;
}

bool RecorderImpl::Deinitialize()
{
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  if (!is_initialized_) return false;

  is_initialized_ = false;
  thread_.join();
  SetLastStatus("Dummy receiver is disconnected.");
  return true;
}

bool RecorderImpl::StartRecording(std::int64_t, const std::string& )
{
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  SetLastStatus("Dummy receiver has started recording.");
  return true;
}

bool RecorderImpl::FlushFrame(std::int64_t, const std::shared_ptr<eCAL::rec::addon::BaseFrame>& frame)
{
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  //std::this_thread::sleep_for(std::chrono::microseconds(50));

  //std::stringstream s;

  //s << "Frame " << std::static_pointer_cast<Frame>(frame)->payload << " flushed" << std::endl;
  //OutputDebugStringA(s.str().c_str());

  SetLastStatus("Frame " + std::static_pointer_cast<Frame>(frame)->payload + " flushed");
  return true;
}

bool RecorderImpl::StopRecording(std::int64_t)
{
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);
  SetLastStatus("Dummy receiver has stopped recording.");
  return true;
}

eCAL::rec::addon::Info RecorderImpl::GetInfo() const
{
  return
  {
    "de.conti.ecal.rec-add-on.dummy",
    "Dummy Add-On",
    "This eCAL recoder add-on is just an example."
  };
}
