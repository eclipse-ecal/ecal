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

/**
 * @brief  Receiver thread for ecal samples
**/

#pragma once

#include "ecal_def.h"
#include "udp_receiver.h"
#include "util/ecal_thread.h"
#include "msg_type.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/ecal.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class CReceiveSlot
{
public:
  CReceiveSlot();
  virtual ~CReceiveSlot();

  int ApplyMessage(const struct SUDPMessage& ecal_message_);
  bool HasFinished() {return((m_recv_mode == rcm_aborted) || (m_recv_mode == rcm_completed));};
  bool HasTimedOut(const std::chrono::duration<double>& diff_time_) {m_timeout += diff_time_; return(m_timeout >= std::chrono::milliseconds(NET_UDP_RECBUFFER_TIMEOUT));};
  int32_t GetMessageTotalLength() {return(m_message_total_len);};
  int32_t GetMessageCurrentLength() {return(m_message_curr_len);};

  virtual int OnMessageCompleted(std::vector<char> &&msg_buffer_) = 0;

protected:
  int OnMessageStart(const struct SUDPMessage& ecal_message_);
  int OnMessageData(const struct SUDPMessage& ecal_message_);

  enum eReceiveMode
  {
    rcm_waiting = 1,
    rcm_reading,
    rcm_aborted,
    rcm_completed
  };

  std::chrono::duration<double> m_timeout;
  std::vector<char> m_recv_buffer;
  eReceiveMode      m_recv_mode;

  int32_t           m_message_id;
  int32_t           m_message_total_num;
  int32_t           m_message_total_len;

  int32_t           m_message_curr_num;
  int32_t           m_message_curr_len;

  eCAL::pb::Sample  m_ecal_sample;
};


class CSampleReceiver
{
  class CSampleReceiveSlot : public CReceiveSlot
  {
  public:
    explicit CSampleReceiveSlot(CSampleReceiver* sample_receiver_);
    virtual ~CSampleReceiveSlot();

    virtual int OnMessageCompleted(std::vector<char> &&msg_buffer_);

  protected:
    CSampleReceiver* m_sample_receiver;
  };

public:
  CSampleReceiver();
  virtual ~CSampleReceiver();

  using HasSampleCallbackT   = std::function<bool(const std::string& sample_name_)>;
  using ApplySampleCallbackT = std::function<void(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType layer_)>;

  void Start(eCAL::SReceiverAttr attr_, HasSampleCallbackT has_sample_callback_, ApplySampleCallbackT apply_sample_callback_, int timeout_);
  void Stop();

  bool AddMultiCastGroup(const char* ipaddr_);
  bool RemMultiCastGroup(const char* ipaddr_);

protected:
  void ReceiveThread(int timeout_);
  void Process(const char* sample_buffer_, size_t sample_buffer_len_);

  HasSampleCallbackT    m_has_sample_callback;
  ApplySampleCallbackT  m_apply_sample_callback;

  eCAL::CUDPReceiver    m_udp_receiver;
  std::thread           m_receive_thread;
  std::atomic<bool>     m_receive_thread_stop;

  using ReceiveSlotMapT = std::unordered_map<int32_t, std::shared_ptr<CSampleReceiveSlot>>;
  ReceiveSlotMapT       m_receive_slot_map;
  std::vector<char>     m_msg_buffer;
  eCAL::pb::Sample      m_ecal_sample;

  std::chrono::steady_clock::time_point m_cleanup_start;
};
