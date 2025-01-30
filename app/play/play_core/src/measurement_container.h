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

#pragma once

#include <string>
#include <map>
#include <memory>

#include <ecal/ecal.h>
#include <ecal/pubsub/publisher.h>
#include <ecalhdf5/eh5_meas.h>

#include "continuity_report.h"

class MeasurementContainer
{
public:
  MeasurementContainer(std::shared_ptr<eCAL::eh5::v2::HDF5Meas> hdf5_meas, const std::string& meas_dir = "", bool use_receive_timestamp = true);
  ~MeasurementContainer();

  void CreatePublishers();
  void CreatePublishers(const std::map<std::string, std::string>& publisher_map);
  void DeInitializePublishers();
  void ClearMessageCounters();
  bool PublishersCreated() const;

  bool PublishFrame(long long index);

  void CalculateEstimatedSizeForChannels();


  //////////////////////////////////////////////////////////////////////////////
  //// Getters                                                              ////
  //////////////////////////////////////////////////////////////////////////////
  long long GetFrameCount() const;
  bool IsUsingReceiveTimestamp() const;
  std::string GetPath() const;

  eCAL::Time::ecal_clock::time_point GetTimestamp(long long index) const;
  std::string GetChannelName(long long index) const;

  std::chrono::nanoseconds GetMeasurementLength() const;
  std::set<std::string> GetChannelNames() const;
  double GetMinTimestampOfChannel(const std::string& channel_name) const;
  double GetMaxTimestampOfChannel(const std::string& channel_name) const;
  std::string GetChannelType(const std::string& channel_name) const;
  std::string GetChannelEncoding(const std::string& channel_name) const;
  size_t GetChannelCumulativeEstimatedSize(const std::string& channel_name) const;
  std::map<std::string, std::string> GetChannelMapping() const;

  std::map<std::string, long long> GetMessageCounters() const;

  long long GetNextEnabledFrameIndex(long long current_index, bool repeat_from_beginning, std::pair<long long, long long> limit_interval) const;

  long long GetNextOccurenceOfChannel(long long current_index, const std::string& source_channel_name, bool repeat_from_beginning, std::pair<long long, long long> limit_interval) const;

  std::chrono::nanoseconds GetTimeBetweenFrames(long long first, long long second) const;

  long long GetNearestIndex(eCAL::Time::ecal_clock::time_point timestamp) const;

  //////////////////////////////////////////////////////////////////////////////
  //// Various                                                              ////
  //////////////////////////////////////////////////////////////////////////////

  std::map<std::string, ContinuityReport> CreateContinuityReport() const;

private:
  void CreateFrameTable();

////////////////////////////////////////////////////////////////////////////////
//// Member Variables                                                       ////
////////////////////////////////////////////////////////////////////////////////

private:
  struct PublisherInfo
  {
    eCAL::CPublisher publisher_;
    long long message_counter_;

    PublisherInfo(const std::string& topic_name, const eCAL::SDataTypeInformation& info_)
      : publisher_(topic_name, info_)
      , message_counter_(0)
    {}
  };

  struct MeasurementFrame
  {
    long long                          id_;
    eCAL::Time::ecal_clock::time_point receive_timestamp_;
    eCAL::Time::ecal_clock::time_point send_timestamp_;
    std::string                        channel_name_;
    long long                          send_id_;
    PublisherInfo*                     publisher_info_;
  };

  std::shared_ptr<eCAL::eh5::v2::HDF5Meas>              hdf5_meas_;
  std::string                                           meas_dir_;
  bool                                                  use_receive_timestamp_;

  std::vector<MeasurementFrame>           frame_table_;
  std::map<std::string, size_t>           total_estimated_channel_size_map_;
  std::map<std::string, PublisherInfo>    publisher_map_;
  bool                                    publishers_initialized_;

  static const size_t                     MIN_SEND_BUFFER_SIZE = 10 * 1024 * 1024;
  size_t                                  allocated_buffer_size_;
  void*                                   send_buffer_;
};

