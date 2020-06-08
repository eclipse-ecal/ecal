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

#include "recorder.h"

#include <unordered_set>


using namespace eCAL::rec::addon;

Recorder::Recorder(RecorderImplBase& impl) : impl_(impl)
{
  pre_buffer_.SetLength(std::chrono::seconds(4)); // TODO: What is default prebuffer length?

  impl_.SetRecordFrameCallback([this](const std::shared_ptr<BaseFrame>& frame) -> bool
  {
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    
    if (!is_initialzed_)
      return false;
    
    auto frame_recorded = false;
    if (pre_buffering_enabled_)
    {
      pre_buffer_.Add(frame);
      frame_recorded = true;
    }

    for (auto& buffer : buffers_)
    {
      auto& job_status = job_statuses_.at(buffer.first);
      if (job_status.state == JobStatus::State::Recording)
      {
        ++job_status.queue_count;
        ++job_status.frame_count;
        buffer.second->Add(frame);
        frame_recorded = true;
      }
    }

    return frame_recorded;
  });
}

Recorder::~Recorder()
{
  if(is_initialzed_)
    Deinitialize();

  impl_.RemoveRecordFrameCallback();
}

bool Recorder::Initialize()
{
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);

  if (is_initialzed_)
  {
    SetLastStatus("Initialization failed. Already initialized.");
    return false;
  }

  if (!impl_.Initialize())
  {
    SetLastStatus("Initialization failed. " + impl_.GetLastStatus());
    return false;
  }

  is_initialzed_ = true;

  SetLastStatus("Initialization succeeded. " + impl_.GetLastStatus());
  return true;
}

bool Recorder::Deinitialize()
{ 
  {
    std::lock_guard<std::shared_timed_mutex> lock(mutex_); // TODO: shared lock ???

    if (!is_initialzed_)
    {
      SetLastStatus("Deinitialization failed. Already deinitialized.");
      return false;
    }

    for (auto job_status_iter = job_statuses_.begin(); job_status_iter != job_statuses_.end(); ++job_status_iter)
    {
      if (job_status_iter->second.state == JobStatus::State::Recording)
      {
        job_status_iter->second.state = JobStatus::State::Flushing;
        job_status_iter->second.healthy = true;
        job_status_iter->second.description = "Stop recording triggered. Flushing buffer.";
        buffers_.at(job_status_iter->first)->Add(std::shared_ptr<BaseFrame>(nullptr));
      }
    }

    is_initialzed_ = false;
  }

  if (!impl_.Deinitialize())
  {
    SetLastStatus("Deinitialization failed. " + impl_.GetLastStatus());
    return false;
  }

  SetLastStatus("Deinitialization succeeded. " + impl_.GetLastStatus());
  return true;
}

bool Recorder::StartRecording(std::int64_t job_id, const std::string& path)
{
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);

  if (!is_initialzed_)
  {
    SetLastStatus("Start recording failed. Add-on not initialized.");
    return false;
  }
  
    if (!job_statuses_.insert(std::make_pair(job_id, JobStatus{ JobStatus::State::NotStarted, true, "", 0, 0 })).second)
    {
      SetLastStatus("Start recording failed. Job ID already exists.");
      return false;
    }
  
    if (!impl_.StartRecording(job_id, path))
    {
      const std::string status = "Start recording failed. " + impl_.GetLastStatus();
      auto& job_status = job_statuses_.at(job_id);
      job_status.healthy = false;
      job_status.description = status;
      SetLastStatus(status);
      return false;
    }
  
  auto buffer = buffers_.insert(std::make_pair(job_id, std::make_shared<TimeLimtedQueue<BaseFrame>>())).first->second;

  if (pre_buffering_enabled_)
    pre_buffer_.Copy(*buffer);

  buffer->SetLength(std::chrono::seconds(1000)); // TODO: Set maximum buffer time
  buffer->SetFlushingCallback([job_id, this](const std::shared_ptr<BaseFrame>& frame) -> bool
  {
    // if receive EOF (nullptr) then stop recording, remove the frame buffer from map and return false to exit the flushing thread
    if (!frame.get()) 
    {
      std::lock_guard<std::shared_timed_mutex> lock(mutex_);

      buffers_.erase(job_id); 
      auto& job_status = job_statuses_.at(job_id);

      if (!impl_.StopRecording(job_id))
      {
        job_status.healthy = false;
        job_status.description = "Stop recording failed. " + impl_.GetLastStatus();
      }
      else
      {
        job_status.healthy = true;
        job_status.description = "Stop recording succeeded. " + impl_.GetLastStatus();
      }
      job_status.state = JobStatus::State::Finished;
      return false;
    }

    if (!impl_.FlushFrame(job_id, frame))
    {
      std::lock_guard<std::shared_timed_mutex> lock(mutex_);

      buffers_.erase(job_id);
      auto& job_status = job_statuses_.at(job_id);
      --job_status.queue_count;
      job_status.description = "Flush frame failed. " + impl_.GetLastStatus();
      job_status.healthy = false;

      return false;
    }

    {
      std::lock_guard<std::shared_timed_mutex> lock(mutex_); // TODO: Shared Lock ?

      auto& job_status = job_statuses_.at(job_id);
      --job_status.queue_count;
      job_status.healthy = true;
      job_status.description = "Flush frame succeeded. " + impl_.GetLastStatus();
      return true;
    }
  });

  auto& job_status = job_statuses_.at(job_id);
  job_status.description = "Start recording succeeded. " + impl_.GetLastStatus();
  job_status.healthy = true;
  job_status.state = JobStatus::State::Recording;
  job_status.queue_count = buffer->Count();
  job_status.frame_count = buffer->Count();
  buffer->EnableFlushing();

  SetLastStatus("Start recording succeeded. " + impl_.GetLastStatus());
  return true;
}

bool Recorder::StopRecording(std::int64_t job_id)
{
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);


  if (!is_initialzed_)
  {
    SetLastStatus("Stop recording failed. Add-on not initialized.");
    return false;
  }


  auto job_status_iter = job_statuses_.find(job_id);
  if (job_status_iter == job_statuses_.end())
  {
    SetLastStatus("Stop recording failed. Job id does not exist.");
    return false;
  }

  if (job_status_iter->second.state != JobStatus::State::Recording)
  {
    SetLastStatus("Stop recording failed. Job is already stopped.");
    return false;
  }

  job_status_iter->second.state = JobStatus::State::Flushing;
  job_status_iter->second.description = "Stop recording triggered. Flushing buffer.";
  buffers_.at(job_id)->Add(std::shared_ptr<BaseFrame>(nullptr));
  SetLastStatus("Stop recording triggered.");

  return true;
}

bool Recorder::SavePrebuffer(std::int64_t job_id, const std::string& path)
{
  if (!pre_buffering_enabled_)
  {
    SetLastStatus("Save prebuffer failed. Prebuffer is not enabled.");
    return false;
  }

  if (!StartRecording(job_id, path))
  {
    SetLastStatus("Save prebuffer failed. " + GetLastStatus());
    return false;
  }

  if (!StopRecording(job_id))
  {
    SetLastStatus("Save prebuffer failed. " + GetLastStatus());
    return false;
  }

  SetLastStatus("Save prebuffer triggered.");
  return true;
}

bool Recorder::SetPrebufferLength(std::chrono::steady_clock::duration duration)
{
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);

  pre_buffer_.SetLength(duration);

  SetLastStatus("Set prebuffer length succeeded.");
  return true;
}

bool Recorder::EnablePrebuffering()
{
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);

  pre_buffering_enabled_ = true;

  SetLastStatus("Enable prebuffering succeeded.");
  return true;
}

bool Recorder::DisablePrebuffering()
{
  std::lock_guard<std::shared_timed_mutex> lock(mutex_);

  pre_buffer_.Clear();
  pre_buffering_enabled_ = false;

  SetLastStatus("Disable prebuffering succeeded.");
  return true;
}

Info Recorder::GetInfo() const
{
  return impl_.GetInfo();
}

std::chrono::steady_clock::duration Recorder::GetPrebufferLength() const
{
  return pre_buffer_.GetLength();
}

std::size_t Recorder::GetPrebufferFrameCount() const
{
  return pre_buffer_.Count();
}

JobStatuses Recorder::GetJobStatuses() const
{
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return job_statuses_;
}
