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

#include <ecal/ecal.h>

#include <algorithm>

#include "ecal_def.h"
#include "rcv_sample.h"


CReceiveSlot::CReceiveSlot()
  : m_timeout(0.0)
  , m_recv_mode(rcm_waiting)
  , m_message_id(0)
  , m_message_total_num(0)
  , m_message_total_len(0)
  , m_message_curr_num(0)
  , m_message_curr_len(0)
{
}

CReceiveSlot::~CReceiveSlot()
{
}

int CReceiveSlot::ApplyMessage(const struct SUDPMessage& ecal_message_)
{
  // reset timeout
  m_timeout = std::chrono::duration<double>(0.0);

  // process current packet
  switch(ecal_message_.header.type)
  {
    // new message started
  case msg_type_header:
    OnMessageStart(ecal_message_);
    break;
    // message data package
  case msg_type_content:
    if(m_recv_mode == rcm_reading)
    {
      OnMessageData(ecal_message_);
    }
    break;
  }

  // we have a complete message in the receive buffer
  if(m_recv_mode == rcm_completed)
  {
    // call complete event
    OnMessageCompleted(std::move(m_recv_buffer));
  }

  return(0);
}

int CReceiveSlot::OnMessageStart(const struct SUDPMessage& ecal_message_)
{
  // store header info
  m_message_id        = ecal_message_.header.id;
  m_message_total_num = ecal_message_.header.num;
  m_message_total_len = ecal_message_.header.len;

  // reset current message states
  m_message_curr_num = 0;
  m_message_curr_len = 0;

  // prepare receive buffer
  m_recv_buffer.clear();
  m_recv_buffer.reserve(static_cast<size_t>(m_message_total_len));

  // switch to reading mode
  m_recv_mode = rcm_reading;

  return(0);
}

int CReceiveSlot::OnMessageData(const struct SUDPMessage& ecal_message_)
{
  // check message id
  if(ecal_message_.header.id != m_message_id)
  {
#ifndef NDEBUG
    // log it
    eCAL::Logging::Log(log_level_debug3, "UDP Sample OnMessageData - WRONG MESSAGE PACKET ID " + std::to_string(ecal_message_.header.id));
#endif
    m_recv_mode = rcm_aborted;
    return(-1);
  }

  // check current packet counter
  if(ecal_message_.header.num != m_message_curr_num)
  {
#ifndef NDEBUG
    // log it
    eCAL::Logging::Log(log_level_debug3, "UDP Sample OnMessageData - WRONG MESSAGE PACKET NUMBER " + std::to_string(ecal_message_.header.num) + " / " + std::to_string(m_message_curr_num));
#endif
    m_recv_mode = rcm_aborted;
    return(-1);
  }

  // check current packet length
  if(ecal_message_.header.len <= 0)
  {
#ifndef NDEBUG
    // log it
    eCAL::Logging::Log(log_level_debug3, "UDP Sample OnMessageData - WRONG MESSAGE PACKET LENGTH " + std::to_string(ecal_message_.header.len));
#endif
    m_recv_mode = rcm_aborted;
    return(-1);
  }

  // copy the message part to the receive message buffer
  //std::copy(ecal_message_.payload, ecal_message_.payload + ecal_message_.header.len, std::back_inserter(m_recv_buffer));
  m_recv_buffer.resize(m_recv_buffer.size() + static_cast<size_t>(ecal_message_.header.len));
  memcpy(m_recv_buffer.data() + m_recv_buffer.size() - static_cast<size_t>(ecal_message_.header.len), ecal_message_.payload, static_cast<size_t>(ecal_message_.header.len));

  // increase packet counter
  m_message_curr_num++;

  // increase current length
  m_message_curr_len += ecal_message_.header.len;

  // last message packet ? -> switch to completed mode
  if(m_message_curr_num == m_message_total_num) m_recv_mode = rcm_completed;

  return(0);
}


CSampleReceiver::CSampleReceiveSlot::CSampleReceiveSlot(CSampleReceiver* sample_receiver_) :
                    m_sample_receiver(sample_receiver_)
{
}

CSampleReceiver::CSampleReceiveSlot::~CSampleReceiveSlot()
{
}

int CSampleReceiver::CSampleReceiveSlot::OnMessageCompleted(std::vector<char> &&msg_buffer_)
{
  if(!m_sample_receiver) return(0);

  // read sample_name size
  unsigned short sample_name_size = ((unsigned short*)(msg_buffer_.data()))[0];
  // read sample_name
  std::string    sample_name(msg_buffer_.data() + sizeof(sample_name_size));

  if(m_sample_receiver->HasSample(sample_name))
  {
    // read sample
    if(!m_ecal_sample.ParseFromArray(msg_buffer_.data() + sizeof(sample_name_size) + sample_name_size, static_cast<int>(msg_buffer_.size() - (sizeof(sample_name_size) + sample_name_size)))) return(0);
#ifndef NDEBUG
    // log it
    eCAL::Logging::Log(log_level_debug3, sample_name + "::UDP Sample Completed");

    // log it
    switch(m_ecal_sample.cmd_type())
    {
    case eCAL::pb::bct_none:
      eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - NONE");
      break;
    case eCAL::pb::bct_set_sample:
      eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - SAMPLE");
      break;
    case eCAL::pb::bct_reg_publisher:
      eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - REGISTER PUBLISHER");
      break;
    case eCAL::pb::bct_reg_subscriber:
      eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - REGISTER SUBSCRIBER");
      break;
    case eCAL::pb::bct_reg_process:
      eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - REGISTER PROCESS");
      break;
    case eCAL::pb::bct_reg_service:
      eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - REGISTER SERVER");
      break;
    case eCAL::pb::bct_reg_client:
      eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - REGISTER CLIENT");
      break;
    default:
      eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - UNKNOWN");
      break;
    }
#endif
    // get layer if this is a payload sample
    eCAL::pb::eTLayerType layer = eCAL::pb::eTLayerType::tl_none;
    if (m_ecal_sample.cmd_type() == eCAL::pb::eCmdType::bct_set_sample)
    {
      if (m_ecal_sample.topic().tlayer_size() > 0)
      {
        layer = m_ecal_sample.topic().tlayer(0).type();
      }
    }
    // apply sample
    m_sample_receiver->ApplySample(m_ecal_sample, layer);
  }

  return(0);
}

CSampleReceiver::CSampleReceiver()
{
  m_msg_buffer.resize(MSG_BUFFER_SIZE);
  m_cleanup_start = std::chrono::steady_clock::now();
}

CSampleReceiver::~CSampleReceiver()
{
}

int CSampleReceiver::Receive(eCAL::CUDPReceiver* sample_receiver_)
{
  if(!sample_receiver_) return(-1);

  // wait for any incoming message
  size_t recv_len = sample_receiver_->Receive(m_msg_buffer.data(), m_msg_buffer.size(), 10);
  if(recv_len > 0)
  {
    return(Process(m_msg_buffer.data(), recv_len));
  }

  return(0);
}

int CSampleReceiver::Process(const char* sample_buffer_, size_t sample_buffer_len_)
{
  // we need at least the header information to start
  if (sample_buffer_len_ < sizeof(SUDPMessageHead)) return(0);

  // cast buffer to udp message struct
  struct SUDPMessage* ecal_message = (struct SUDPMessage*)sample_buffer_;

  // check for eCAL 4.x header
  if ((ecal_message->header.head[0] == 'e')
    && (ecal_message->header.head[1] == 'C')
    && (ecal_message->header.head[2] == 'A')
    && (ecal_message->header.head[3] == 'L')
    )
  {
    eCAL::Logging::Log(log_level_warning, "Received eCAL 4 traffic");
    return(0);
  }

  // check for valid header
  else if ( (ecal_message->header.head[0] != 'E')
    || (ecal_message->header.head[1] != 'C')
    || (ecal_message->header.head[2] != 'A')
    || (ecal_message->header.head[3] != 'L')
    )
  {
    eCAL::Logging::Log(log_level_warning, "Received invalid traffic (eCAL Header missing)");
    return(0);
  }

  // check integrity
  switch (ecal_message->header.type)
  {
  case msg_type_header:
    break;
  case msg_type_content:
  case msg_type_header_with_content:
    if (sample_buffer_len_ < sizeof(SUDPMessageHead) + static_cast<size_t>(ecal_message->header.len))
      return(0);
    break;
  default:
    return(0);
  }

#ifndef NDEBUG
  // log it
  switch (ecal_message->header.type)
  {
  case msg_type_header_with_content:
    eCAL::Logging::Log(log_level_debug4, "UDP Sample Received - HEADER_WITH_CONTENT");
    break;
  case msg_type_header:
    eCAL::Logging::Log(log_level_debug4, "UDP Sample Received - HEADER");
    break;
  case msg_type_content:
    eCAL::Logging::Log(log_level_debug4, "UDP Sample Received - CONTENT");
    break;
  }
#endif

  switch (ecal_message->header.type)
  {
  case msg_type_header_with_content:
  {
    // read sample_name size
    unsigned short sample_name_size = 0;
    memcpy(&sample_name_size, ecal_message->payload, 2);
    // read sample_name
    std::string sample_name = ecal_message->payload + sizeof(sample_name_size);

    if (HasSample(sample_name))
    {
      // read sample
      if (!m_ecal_sample.ParseFromArray(ecal_message->payload + static_cast<size_t>(sizeof(sample_name_size) + sample_name_size), static_cast<int>(static_cast<size_t>(ecal_message->header.len) - (sizeof(sample_name_size) + sample_name_size)))) return(0);

#ifndef NDEBUG
      // log it
      eCAL::Logging::Log(log_level_debug3, sample_name + "::UDP Sample Completed");

      // log it
      switch (m_ecal_sample.cmd_type())
      {
      case eCAL::pb::bct_none:
        eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - NONE");
        break;
      case eCAL::pb::bct_set_sample:
        eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - SAMPLE");
        break;
      case eCAL::pb::bct_reg_publisher:
        eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - REGISTER PUBLISHER");
        break;
      case eCAL::pb::bct_reg_subscriber:
        eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - REGISTER SUBSCRIBER");
        break;
      case eCAL::pb::bct_reg_process:
        eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - REGISTER PROCESS");
        break;
      case eCAL::pb::bct_reg_service:
        eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - REGISTER SERVICE");
        break;
      case eCAL::pb::bct_reg_client:
        eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - REGISTER CLIENT");
        break;
      default:
        eCAL::Logging::Log(log_level_debug4, sample_name + "::UDP Sample Command Type - UNKNOWN");
        break;
      }
#endif
      // get layer if this is a payload sample
      eCAL::pb::eTLayerType layer = eCAL::pb::eTLayerType::tl_none;
      if (m_ecal_sample.cmd_type() == eCAL::pb::eCmdType::bct_set_sample)
      {
        if (m_ecal_sample.topic().tlayer_size() > 0)
        {
          layer = m_ecal_sample.topic().tlayer(0).type();
        }
      }
      // apply sample
      ApplySample(m_ecal_sample, layer);
    }
  }
  break;
  // if we have a header only package 
  // we create a receive slot and apply it to the receive slot map
  // to process the following data packages
  // we do not know the name here unfortunately
  // so we have to wait for the first payload package :-(
  case msg_type_header:
  {
    // create new receive slot
    std::shared_ptr<CSampleReceiveSlot> receive_slot(nullptr);
    receive_slot = std::make_shared<CSampleReceiveSlot>(CSampleReceiveSlot(this));
    m_receive_slot_map[ecal_message->header.id] = receive_slot;
    // apply message
    receive_slot->ApplyMessage(*ecal_message);
  }
  break;
  // if we have a payload package 
  // we check for an existing receive slot and apply the data to it
  case msg_type_content:
  {
    // first data package ?
    if (ecal_message->header.num == 0)
    {
      // read sample_name size
      unsigned short sample_name_size = 0;
      memcpy(&sample_name_size, ecal_message->payload, 2);
      // read sample_name
      std::string sample_name = ecal_message->payload + sizeof(sample_name_size);

      // remove the matching slot if we are not interested in this sample
      if (!HasSample(sample_name))
      {
        auto riter = m_receive_slot_map.find(ecal_message->header.id);
        if (riter != m_receive_slot_map.end())
        {
#ifndef NDEBUG
          // log timeouted slot
          eCAL::Logging::Log(log_level_debug3, "CSampleReceiver::Receive - DISCARD PACKAGE FOR TOPIC: " + sample_name);
#endif
          m_receive_slot_map.erase(riter);
          break;
        }
      }
    }

    // process data package
    auto iter = m_receive_slot_map.find(ecal_message->header.id);
    if (iter != m_receive_slot_map.end())
    {
      // apply message
      iter->second->ApplyMessage(*ecal_message);
    }
  }
  break;
  default:
    break;
  }

  // cleanup finished or zombie received slots
  auto diff_time = std::chrono::steady_clock::now() - m_cleanup_start;
  std::chrono::duration<double> step_time = std::chrono::milliseconds(NET_UDP_RECBUFFER_CLEANUP);
  if (diff_time > step_time)
  {
    m_cleanup_start = std::chrono::steady_clock::now();

    for (ReceiveSlotMapT::iterator riter = m_receive_slot_map.begin(); riter != m_receive_slot_map.end();)
    {
      bool finished = riter->second->HasFinished();
      bool timeouted = riter->second->HasTimedOut(step_time);
      if (finished || timeouted)
      {
#ifndef NDEBUG
        int32_t total_len = riter->second->GetMessageTotalLength();
        int32_t current_len = riter->second->GetMessageCurrentLength();
#endif
        riter = m_receive_slot_map.erase(riter);
#ifndef NDEBUG
        // log timeouted slot
        if (timeouted)
        {
          eCAL::Logging::Log(log_level_debug3, "CSampleReceiver::Receive - TIMEOUT (TotalLength / CurrentLength):  " + std::to_string(total_len) + " / " + std::to_string(current_len));
        }
#endif
      }
      else
      {
        ++riter;
      }
    }
  }

  return(static_cast<int>(sample_buffer_len_));
}
