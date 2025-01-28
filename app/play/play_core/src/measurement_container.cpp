/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include "measurement_container.h"

#include <ecal/util.h>
#include <ecalhdf5/eh5_meas.h>

#include <algorithm>
#include <math.h>
#include <stdlib.h>

MeasurementContainer::MeasurementContainer(std::shared_ptr<eCAL::eh5::v2::HDF5Meas> hdf5_meas, const std::string& meas_dir, bool use_receive_timestamp)
  : hdf5_meas_             (hdf5_meas)
  , meas_dir_              (meas_dir)
  , use_receive_timestamp_ (use_receive_timestamp)
  , publishers_initialized_(false)
  , send_buffer_           (nullptr)
{
  send_buffer_           = malloc(MIN_SEND_BUFFER_SIZE);
  allocated_buffer_size_ = MIN_SEND_BUFFER_SIZE;

  // Create a table of all frames, sorted by their timestamps
  CreateFrameTable();
}

MeasurementContainer::~MeasurementContainer()
{
  DeInitializePublishers();
  free(send_buffer_);
}

void MeasurementContainer::CreateFrameTable()
{
  auto channel_names = hdf5_meas_->GetChannelNames();
  for (auto& channel_name : channel_names)
  {
    eCAL::experimental::measurement::base::EntryInfoSet entry_info_set;
    if (hdf5_meas_->GetEntriesInfo(channel_name, entry_info_set))
    {
      for (auto& entry_info : entry_info_set)
      {
        MeasurementFrame frame_entry;

        frame_entry.id_                = entry_info.ID;
        frame_entry.channel_name_      = channel_name;
        frame_entry.receive_timestamp_ = eCAL::Time::ecal_clock::time_point(std::chrono::microseconds(entry_info.RcvTimestamp));
        frame_entry.send_timestamp_    = eCAL::Time::ecal_clock::time_point(std::chrono::microseconds(entry_info.SndTimestamp));
        frame_entry.send_id_           = entry_info.SndID;
        frame_entry.publisher_info_    = nullptr;

        frame_table_.push_back(frame_entry);
      }
    }
  }

  if (use_receive_timestamp_)
  {
    std::sort(frame_table_.begin(), frame_table_.end(), [](const MeasurementFrame& e1, const MeasurementFrame& e2) {return e1.receive_timestamp_ < e2.receive_timestamp_; });
  }
  else
  {
    std::sort(frame_table_.begin(), frame_table_.end(), [](const MeasurementFrame& e1, const MeasurementFrame& e2) {return e1.send_timestamp_ < e2.send_timestamp_; });
  }
}

void MeasurementContainer::CalculateEstimatedSizeForChannels()
{
  total_estimated_channel_size_map_.clear();
  auto channel_names = hdf5_meas_->GetChannelNames();
  for (auto& channel_name : channel_names)
  {
    eCAL::experimental::measurement::base::EntryInfoSet entry_info_set;
    if (hdf5_meas_->GetEntriesInfo(channel_name, entry_info_set))
    {
      auto size = entry_info_set.size();
      size_t calculatedStep = size / 5;
      size_t step = (calculatedStep > 0) ? calculatedStep : 1;
      size_t sum = 0;
      uint8_t additions = 0;

      for (size_t i = 0; i < size; i += step)
      {
        size_t entry_size = 0;
        auto id = (*std::next(entry_info_set.begin(), i)).ID;
        hdf5_meas_->GetEntryDataSize(id, entry_size);
        ++additions;
        sum += entry_size;
      }

      size_t average = (additions == 0) ? 0 : (sum / additions);
      total_estimated_channel_size_map_[channel_name] = (average * size);
    }
  }
}

void MeasurementContainer::CreatePublishers()
{
  // Create an identity-mapping of all channels
  std::map<std::string, std::string> publisher_map;
  for (const std::string& channel : GetChannelNames())
  {
    publisher_map[channel] = channel;
  }

  // Create publishers with the identity-mapping
  CreatePublishers(publisher_map);
}

void MeasurementContainer::CreatePublishers(const std::map<std::string, std::string>& publisher_map)
{
  // De-Initialize old publishers
  DeInitializePublishers();

  // Create new publishers
  for (const auto& channel_mapping : publisher_map)
  {
    auto topic_info        = hdf5_meas_->GetChannelDataTypeInformation(channel_mapping.first);
    eCAL::SDataTypeInformation data_type_info;
    data_type_info.name = topic_info.name;
    data_type_info.encoding = topic_info.encoding;
    data_type_info.descriptor = topic_info.descriptor;
    publisher_map_.emplace(channel_mapping.first, PublisherInfo(channel_mapping.second, data_type_info));
  }

  // Assign publishers to entries
  for (auto& entry : frame_table_)
  {
    auto publisher_it = publisher_map_.find(entry.channel_name_);
    if (publisher_it != publisher_map_.end())
    {
      entry.publisher_info_ = &(publisher_it->second);
    }
  }

  publishers_initialized_ = true;
}

void MeasurementContainer::DeInitializePublishers()
{
  // Clear the publisher map
  publisher_map_.clear();

  // Remove pointers to publishers from all frames
  for (auto& entry : frame_table_)
  {
    entry.publisher_info_ = nullptr;
  }

  publishers_initialized_ = false;
}


void MeasurementContainer::ClearMessageCounters()
{
  for (auto& publisher_info : publisher_map_)
  {
    publisher_info.second.message_counter_ = 0;
  }
}

bool MeasurementContainer::PublishersCreated() const
{
  return publishers_initialized_;
}

bool MeasurementContainer::PublishFrame(long long index)
{
  // Check that the user created the publishers before publishing a frame
  if (!publishers_initialized_ || (index < 0) || index >= GetFrameCount())
    return false;

  if (frame_table_[index].publisher_info_)
  {
    size_t data_size;
    if (hdf5_meas_->GetEntryDataSize(frame_table_[index].id_, data_size))
    {
      if (data_size > allocated_buffer_size_)
      {
        void* temp = realloc(send_buffer_, data_size);
        if (temp == nullptr)
        {
          return false;
        }
        else
        {
          send_buffer_ = temp;
        }
        allocated_buffer_size_ = data_size;
      }
      if (hdf5_meas_->GetEntryData(frame_table_[index].id_, send_buffer_))
      {
        long long timestamp_usecs = -1;
        if (use_receive_timestamp_)
        {
          timestamp_usecs = std::chrono::duration_cast<std::chrono::microseconds>(frame_table_[index].receive_timestamp_.time_since_epoch()).count();
        }
        else
        {
          timestamp_usecs = std::chrono::duration_cast<std::chrono::microseconds>(frame_table_[index].send_timestamp_.time_since_epoch()).count();
        }
        // this is not supported by the eCAL v6 API
        //frame_table_[index].publisher_info_->publisher_.SetID(frame_table_[index].send_id_);
        frame_table_[index].publisher_info_->publisher_.Send(send_buffer_, data_size, timestamp_usecs);
        frame_table_[index].publisher_info_->message_counter_++;
        return true;
      }
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////
//// Getters                                                                ////
////////////////////////////////////////////////////////////////////////////////

long long MeasurementContainer::GetFrameCount() const
{
  return (long long)frame_table_.size();
}

bool MeasurementContainer::IsUsingReceiveTimestamp() const
{
  return use_receive_timestamp_;
}

std::string MeasurementContainer::GetPath() const
{
  return meas_dir_;
}

eCAL::Time::ecal_clock::time_point MeasurementContainer::GetTimestamp(long long index) const
{
  if ((index >= 0) && (index < GetFrameCount()))
  {
    if (use_receive_timestamp_)
    {
      return frame_table_[index].receive_timestamp_;
    }
    else
    {
      return frame_table_[index].send_timestamp_;
    }
  }
  else
  {
    return eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));
  }
}

std::string MeasurementContainer::GetChannelName(long long index) const
{
  if ((index >= 0) && (index < GetFrameCount()))
  {
    return frame_table_[index].channel_name_;
  }
  else
  {
    return "";
  }
}

std::chrono::nanoseconds MeasurementContainer::GetMeasurementLength() const
{
  return GetTimestamp(GetFrameCount() - 1) - GetTimestamp(0);
}

std::set<std::string> MeasurementContainer::GetChannelNames() const
{
  return hdf5_meas_->GetChannelNames();
}

double MeasurementContainer::GetMinTimestampOfChannel(const std::string& channel_name) const
{
  auto minTimestamp = eCAL::Time::ecal_clock::time_point(std::chrono::microseconds(hdf5_meas_->GetMinTimestamp(channel_name)));
  auto relativeMinTimestamp = std::chrono::duration_cast<std::chrono::duration<double>>(minTimestamp - GetTimestamp(0)).count();
  double roundedRelativeMinTimestamp = round((relativeMinTimestamp * 1000.0)) / 1000.0;

  return roundedRelativeMinTimestamp;
}

double MeasurementContainer::GetMaxTimestampOfChannel(const std::string& channel_name) const
{
  auto maxTimestamp = eCAL::Time::ecal_clock::time_point(std::chrono::microseconds(hdf5_meas_->GetMaxTimestamp(channel_name)));
  auto relativeMaxTimestamp = std::chrono::duration_cast<std::chrono::duration<double>>(maxTimestamp - GetTimestamp(0)).count();
  double roundedRelativeMaxTimestamp = round((relativeMaxTimestamp * 1000.0)) / 1000.0;

  return roundedRelativeMaxTimestamp;
}

std::string MeasurementContainer::GetChannelType(const std::string& channel_name) const
{
  return hdf5_meas_->GetChannelDataTypeInformation(channel_name).name;
}

std::string MeasurementContainer::GetChannelEncoding(const std::string& channel_name) const
{
  return hdf5_meas_->GetChannelDataTypeInformation(channel_name).encoding;
}

size_t MeasurementContainer::GetChannelCumulativeEstimatedSize(const std::string& channel_name) const
{
  auto it = total_estimated_channel_size_map_.find(channel_name);
  if (it != total_estimated_channel_size_map_.end())
    return it->second;

  return 0;
}

std::map<std::string, std::string> MeasurementContainer::GetChannelMapping() const
{
  std::map<std::string, std::string> channel_mapping;

  for (auto& publisher : publisher_map_)
  {
    channel_mapping.emplace(publisher.first, publisher.second.publisher_.GetTopicName());
  }

  return channel_mapping;
}

std::map<std::string, long long> MeasurementContainer::GetMessageCounters() const
{
  std::map<std::string, long long> message_counters;
  for (auto& publisher_info : publisher_map_)
  {
    message_counters.emplace(publisher_info.first, publisher_info.second.message_counter_);
  }
  return message_counters;
}

long long MeasurementContainer::GetNextEnabledFrameIndex(long long current_index, bool repeat_from_beginning, std::pair<long long, long long> limit_interval) const
{
  // Check that the user created the publishers before looking for enabled indices
  if (!publishers_initialized_)
    return -1;

  // Search from current_index to the end
  for (long long i = std::max(current_index, limit_interval.first) + 1; i <= std::min(limit_interval.second, GetFrameCount() - 1); i++)
  {
    if (frame_table_[i].publisher_info_)
    {
      return i;
    }
  }

  // Search from the beginning to current_index
  if (repeat_from_beginning)
  {
    for (long long i = std::max(0LL, limit_interval.first); i <= std::min(std::min(current_index, limit_interval.second), GetFrameCount() - 1); i++)
    {
      if (frame_table_[i].publisher_info_)
      {
        return i;
      }
    }
  }

  // Nothing found
  return -1;
}

long long MeasurementContainer::GetNextOccurenceOfChannel(long long current_index, const std::string& source_channel_name, bool repeat_from_beginning, std::pair<long long, long long> limit_interval) const
{
  // Search from current_index to the end
  for (long long i = std::max(current_index, limit_interval.first) + 1; i <= std::min(limit_interval.second, GetFrameCount() - 1); i++)
  {
    if (frame_table_[i].channel_name_ == source_channel_name)
    {
      return i;
    }
  }

  // Search from the beginning to current_index
  if (repeat_from_beginning)
  {
    for (long long i = std::max(0LL, limit_interval.first); i <= std::min(std::min(current_index, limit_interval.second), GetFrameCount() - 1); i++)
    {
      if (frame_table_[i].channel_name_ == source_channel_name)
      {
        return i;
      }
    }
  }

  // Nothing found
  return - 1;
}

std::chrono::nanoseconds MeasurementContainer::GetTimeBetweenFrames(long long first, long long second) const
{
  if (second >= first)
  {
    return GetTimestamp(second) - GetTimestamp(first);
  }
  else
  {
    // The second timestamp is before the first one => we repeat from the beginning
    return (GetTimestamp(GetFrameCount() - 1) - GetTimestamp(second)) + (GetTimestamp(first) - GetTimestamp(0));
  }
}

long long MeasurementContainer::GetNearestIndex(eCAL::Time::ecal_clock::time_point timestamp) const
{
  if (frame_table_.size() < 1)
  {
    return -1;
  }

  if (timestamp <= GetTimestamp(0))
  {
    return 0;
  }

  for (int i = 0; i < GetFrameCount() - 1; i++)
  {
    if ((GetTimestamp(i) <= timestamp) && (timestamp <= GetTimestamp(i + 1)))
    {
      if ((timestamp - GetTimestamp(i)) <= (GetTimestamp(i + 1) - timestamp))
      {
        return i;
      }
      else
      {
        return i + 1;
      }
    }
  }
  return GetFrameCount() - 1;
}

////////////////////////////////////////////////////////////////////////////////
//// Getters                                                                ////
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, ContinuityReport> MeasurementContainer::CreateContinuityReport() const
{
  std::map<std::string, ContinuityReport> continuity_report;

  auto channel_names = hdf5_meas_->GetChannelNames();
  for (auto& channel_name : channel_names)
  {
    eCAL::experimental::measurement::base::EntryInfoSet entry_info_set;
    if (hdf5_meas_->GetEntriesInfo(channel_name, entry_info_set))
    {

      if (!entry_info_set.empty())
      {
        bool single_source = true;
        for (auto entry_it = entry_info_set.begin(); entry_it != entry_info_set.end(); entry_it++)
        {
          auto this_it = entry_it;
          auto next_it = std::next(entry_it, 1);
          
          if (!(next_it == entry_info_set.end()))
          {
            if (next_it->SndClock <= this_it->SndClock)
            {
              single_source = false;
              break;
            }
          }
        }

        long long expected_frame_count = entry_info_set.rbegin()->SndClock - entry_info_set.begin()->SndClock + 1;
        long long existing_frame_count = entry_info_set.size();

        if (single_source)
        {
          continuity_report.emplace(channel_name, ContinuityReport(expected_frame_count, existing_frame_count));
        }
        else
        {
          continuity_report.emplace(channel_name, ContinuityReport(-1LL, existing_frame_count));
        }
      }
      else
      {
        continuity_report.emplace(channel_name, ContinuityReport(0LL, 0LL));
      }
    }
  }

  return continuity_report;
}