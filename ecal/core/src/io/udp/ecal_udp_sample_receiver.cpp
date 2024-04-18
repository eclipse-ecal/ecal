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
 * @brief  UDP sample receiver to receive messages of type eCAL::pb::Sample
**/

#include "ecal_udp_sample_receiver.h"
#include "io/udp/fragmentation/msg_type.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ecal/ecal_log.h>
#include <memory>
#include <string>
#include <vector>

namespace eCAL
{
  namespace UDP
  {
    CSampleReceiver::CSampleDefragmentation::CSampleDefragmentation(CSampleReceiver* sample_receiver_)
      : m_sample_receiver(sample_receiver_)
    {
    }

    CSampleReceiver::CSampleDefragmentation::~CSampleDefragmentation() = default;

    int CSampleReceiver::CSampleDefragmentation::OnMessageCompleted(std::vector<char>&& msg_buffer_)
    {
      if (m_sample_receiver == nullptr) return(0);

      // read sample_name size
      const unsigned short sample_name_size = ((unsigned short*)(msg_buffer_.data()))[0];

      // check for damaged data
      if (sample_name_size > msg_buffer_.size())
      {
        std::cerr << "CSampleReceiver: Received damaged data. Wrong sample name size." << '\n';
        return(0);
      }

      // read sample_name
      const std::string sample_name(msg_buffer_.data() + sizeof(sample_name_size));

      // calculate payload offset
      auto payload_offset = sizeof(sample_name_size) + sample_name_size;

      // check for damaged data
      if (payload_offset > msg_buffer_.size())
      {
        std::cerr << "CSampleReceiverAsio: Received damaged data. Wrong payload buffer offset." << '\n';
        return(0);
      }

      if (m_sample_receiver->m_has_sample_callback(sample_name))
      {
        // read sample
        if (!m_ecal_sample.ParseFromArray(msg_buffer_.data() + payload_offset, static_cast<int>(msg_buffer_.size() - (sizeof(sample_name_size) + sample_name_size)))) return(0);
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
        m_sample_receiver->m_apply_sample_callback(m_ecal_sample, layer);
      }

      return(0);
    }

    CSampleReceiver::CSampleReceiver(const IO::UDP::SReceiverAttr& attr_, HasSampleCallbackT has_sample_callback_, ApplySampleCallbackT apply_sample_callback_) :
      m_has_sample_callback(has_sample_callback_), m_apply_sample_callback(apply_sample_callback_)
    {
      // create udp receiver
      m_udp_receiver.Create(attr_);

      // allocate receive buffer
      m_msg_buffer.resize(MSG_BUFFER_SIZE);

      // start receiver thread
      m_udp_receiver_thread = std::make_shared<eCAL::CCallbackThread>(std::bind(&CSampleReceiver::ReceiveThread, this));
      m_udp_receiver_thread->start(std::chrono::milliseconds(0));

      m_cleanup_start = std::chrono::steady_clock::now();
    }

    CSampleReceiver::~CSampleReceiver()
    {
      // stop receiver thread
      m_udp_receiver_thread->stop();

      // destroy udp receiver
      m_udp_receiver.Destroy();
    }

    bool CSampleReceiver::AddMultiCastGroup(const char* ipaddr_)
    {
      return m_udp_receiver.AddMultiCastGroup(ipaddr_);
    }

    bool CSampleReceiver::RemMultiCastGroup(const char* ipaddr_)
    {
      return m_udp_receiver.RemMultiCastGroup(ipaddr_);
    }

    void CSampleReceiver::ReceiveThread()
    {
      // wait for any incoming message
      const size_t recv_len = m_udp_receiver.Receive(m_msg_buffer.data(), m_msg_buffer.size(), CMN_UDP_RECEIVE_THREAD_CYCLE_TIME_MS);
      if (recv_len > 0)
      {
        Process(m_msg_buffer.data(), recv_len);
      }
    }

    void CSampleReceiver::Process(const char* sample_buffer_, size_t sample_buffer_len_)
    {
      // we need at least the header information to start
      if (sample_buffer_len_ < sizeof(IO::UDP::SUDPMessageHead)) return;

      // cast buffer to udp message struct
      struct IO::UDP::SUDPMessage* ecal_message = (struct IO::UDP::SUDPMessage*)sample_buffer_;

      // check for eCAL 4.x header
      if (
           (ecal_message->header.head[0] == 'e')
        && (ecal_message->header.head[1] == 'C')
        && (ecal_message->header.head[2] == 'A')
        && (ecal_message->header.head[3] == 'L')
        )
      {
        eCAL::Logging::Log(log_level_warning, "Received eCAL 4 traffic");
        return;
      }

      // check for valid header
      else if (
           (ecal_message->header.head[0] != 'E')
        || (ecal_message->header.head[1] != 'C')
        || (ecal_message->header.head[2] != 'A')
        || (ecal_message->header.head[3] != 'L')
        )
      {
        eCAL::Logging::Log(log_level_warning, "Received invalid traffic (eCAL Header missing)");
        return;
      }

      // check integrity
      switch (ecal_message->header.type)
      {
      case IO::UDP::msg_type_header:
        break;
      case IO::UDP::msg_type_content:
      case IO::UDP::msg_type_header_with_content:
        if (sample_buffer_len_ < sizeof(IO::UDP::SUDPMessageHead) + static_cast<size_t>(ecal_message->header.len))
          return;
        break;
      default:
        return;
      }

#ifndef NDEBUG
      // log it
      switch (ecal_message->header.type)
      {
      case IO::UDP::msg_type_header_with_content:
        eCAL::Logging::Log(log_level_debug4, "UDP Sample Received - HEADER_WITH_CONTENT");
        break;
      case IO::UDP::msg_type_header:
        eCAL::Logging::Log(log_level_debug4, "UDP Sample Received - HEADER");
        break;
      case IO::UDP::msg_type_content:
        eCAL::Logging::Log(log_level_debug4, "UDP Sample Received - CONTENT");
        break;
      }
#endif

      switch (ecal_message->header.type)
      {
      case IO::UDP::msg_type_header_with_content:
      {
        // read sample_name size
        unsigned short sample_name_size = 0;
        memcpy(&sample_name_size, ecal_message->payload, 2);
        // read sample_name
        const std::string sample_name = ecal_message->payload + sizeof(sample_name_size);

        if (m_has_sample_callback(sample_name))
        {
          // read sample
          if (!m_ecal_sample.ParseFromArray(ecal_message->payload + static_cast<size_t>(sizeof(sample_name_size) + sample_name_size), static_cast<int>(static_cast<size_t>(ecal_message->header.len) - (sizeof(sample_name_size) + sample_name_size)))) return;

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
          m_apply_sample_callback(m_ecal_sample, layer);
        }
      }
      break;
      // if we have a header only package 
      // we create a receive defragmentation buffer and apply it to the receive defragmentation map
      // to process the following data packages
      // we do not know the name here unfortunately
      // so we have to wait for the first payload package :-(
      case IO::UDP::msg_type_header:
      {
        // create new receive defragmentation buffer
        std::shared_ptr<CSampleDefragmentation> receive_defragmentation_buf(nullptr);
        receive_defragmentation_buf = std::make_shared<CSampleDefragmentation>(CSampleDefragmentation(this));
        m_defrag_sample_map[ecal_message->header.id] = receive_defragmentation_buf;
        // apply message
        receive_defragmentation_buf->ApplyMessage(*ecal_message);
      }
      break;
      // if we have a payload package 
      // we check for an existing receive defragmentation buffer and apply the data to it
      case IO::UDP::msg_type_content:
      {
        // first data package ?
        if (ecal_message->header.num == 0)
        {
          // read sample_name size
          unsigned short sample_name_size = 0;
          memcpy(&sample_name_size, ecal_message->payload, 2);
          // read sample_name
          const std::string sample_name = ecal_message->payload + sizeof(sample_name_size);

          // remove the matching defragmentation buffer if we are not interested in this sample
          if (!m_has_sample_callback(sample_name))
          {
            auto riter = m_defrag_sample_map.find(ecal_message->header.id);
            if (riter != m_defrag_sample_map.end())
            {
#ifndef NDEBUG
              // log timeouted defragmentation buffers
              eCAL::Logging::Log(log_level_debug3, "CUDPSampleReceiver::Receive - DISCARD PACKAGE FOR TOPIC: " + sample_name);
#endif
              m_defrag_sample_map.erase(riter);
              break;
            }
          }
        }

        // process data package
        auto iter = m_defrag_sample_map.find(ecal_message->header.id);
        if (iter != m_defrag_sample_map.end())
        {
          // apply message
          iter->second->ApplyMessage(*ecal_message);
        }
      }
      break;
      default:
        break;
      }

      // cleanup finished or zombie received defragmentation buffers
      auto diff_time = std::chrono::steady_clock::now() - m_cleanup_start;
      const std::chrono::duration<double> step_time = std::chrono::milliseconds(NET_UDP_RECBUFFER_CLEANUP);
      if (diff_time > step_time)
      {
        m_cleanup_start = std::chrono::steady_clock::now();

        for (SampleDefragmentationMapT::iterator riter = m_defrag_sample_map.begin(); riter != m_defrag_sample_map.end();)
        {
          const bool finished = riter->second->HasFinished();
          const bool timeouted = riter->second->HasTimedOut(step_time);
          if (finished || timeouted)
          {
#ifndef NDEBUG
            const int32_t total_len = riter->second->GetMessageTotalLength();
            const int32_t current_len = riter->second->GetMessageCurrentLength();
#endif
            riter = m_defrag_sample_map.erase(riter);
#ifndef NDEBUG
            // log timeouted defragmentation buffer
            if (timeouted)
            {
              eCAL::Logging::Log(log_level_debug3, "CUDPSampleReceiver::Receive - TIMEOUT (TotalLength / CurrentLength):  " + std::to_string(total_len) + " / " + std::to_string(current_len));
            }
#endif
          }
          else
          {
            ++riter;
          }
        }
      }
    }
  }
}
