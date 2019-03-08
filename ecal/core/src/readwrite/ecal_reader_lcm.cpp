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
 * @brief  eCAL lcm reader
**/

#include "ecal_config_hlp.h"
#include "ecal_global_accessors.h"
#include "mon/ecal_monitoring_def.h"

#include "pubsub/ecal_subgate.h"
#include "readwrite/ecal_reader_lcm.h"

namespace eCAL
{
  template<> std::shared_ptr<CLcmLayer> CReaderLayer<CLcmLayer>::layer(nullptr);

  //////////////////////////////////////////////////////////////////
  // CLcmSampleReceiver
  //////////////////////////////////////////////////////////////////
  static void lcm_subscriber(lcmlite_sub_t* /*lcm_*/, const char* channel_, const void* buf_, int len_, void* user_)
  {
    eCAL::pb::Sample ecal_sample;
    auto ecal_sample_mutable_topic = ecal_sample.mutable_topic();

    // create new sample
    ecal_sample.set_cmd_type(eCAL::pb::bct_set_sample);
    ecal_sample_mutable_topic->set_hname("");
    ecal_sample_mutable_topic->set_tname(channel_);

    // set layer
    auto layer = ecal_sample_mutable_topic->add_tlayer();
    layer->set_type(eCAL::pb::eTLayerType::tl_ecal_udp_mc);
    layer->set_confirmed(true);

    // append content
    auto ecal_sample_mutable_content = ecal_sample.mutable_content();
    ecal_sample_mutable_content->set_clock(0);
    ecal_sample_mutable_content->set_time(Time::GetMicroSeconds());
    ecal_sample_mutable_content->set_size(static_cast<google::protobuf::int32>(len_));
    ecal_sample_mutable_content->set_payload(buf_, static_cast<size_t>(len_));

    if (g_subgate()) g_subgate()->ApplySample(ecal_sample, eCAL::pb::eTLayerType::tl_lcm);
    static_cast<CDataReaderLCM*>(user_)->ApplyLcmPublisher(std::string(channel_), static_cast<size_t>(len_));
  }

  CDataReaderLCM::CDataReaderLCM()
  {
    SReceiverAttr attr;
    attr.ipaddr     = NET_UDP_MULTICAST_GROUP_LCM;
    attr.port       = NET_UDP_MULTICAST_PORT_LCM;
    attr.loopback   = true;
    attr.rcvbuf     = eCALPAR(NET, UDP_MULTICAST_RCVBUF);
    attr.local_only = false;
    m_receiver.Create(attr);

    m_msg_buffer.resize(LCM3_MAX_PACKET_SIZE);

    m_lcm = std::make_shared<lcmlite_sub_t>();
    lcmlite_sub_init(m_lcm.get());

    static char wcard[]         = ".*";
    m_lcm_subscription.channel  = wcard;
    m_lcm_subscription.callback = lcm_subscriber;
    m_lcm_subscription.user     = this;
    lcmlite_subscribe(m_lcm.get(), &m_lcm_subscription);
  }

  int CDataReaderLCM::ApplyLcmPublisher(const std::string & topic_name_, size_t topic_size_)
  {
    std::lock_guard<std::mutex> lock(m_lcm_pub_map_sync);
    auto topic_iter = m_lcm_pub_map.find(topic_name_);
    if (topic_iter == m_lcm_pub_map.end())
    {
      ExtTopic topic;
      topic.direction = ExtTopic::publisher;
      topic.tsize = static_cast<int>(topic_size_);
      m_lcm_pub_map[topic_name_] = topic;
    }
    else
    {
      auto ttime = std::chrono::steady_clock::now();
      auto ttime_old = topic_iter->second.ttime;

      topic_iter->second.tsize = static_cast<int>(topic_size_);
      topic_iter->second.tclock = topic_iter->second.tclock++;
      topic_iter->second.ttime = ttime;
      topic_iter->second.dtime = ttime - ttime_old;
      if ((ttime_old != std::chrono::steady_clock::time_point(std::chrono::nanoseconds(0)))
        && ((ttime - ttime_old) > std::chrono::nanoseconds(0)))
      {
        topic_iter->second.dfreq = static_cast<int>(1000000000 / std::chrono::duration_cast<std::chrono::microseconds>(ttime - ttime_old).count());
      }
      topic_iter->second.updated = true;
    }
    return 0;
  }

  int CDataReaderLCM::Monitor()
  {
    auto time = std::chrono::steady_clock::now();

    // remove outdated topics
    ExtMapT topic_map;
    {
      std::lock_guard<std::mutex> lock(m_lcm_pub_map_sync);
      auto iter = m_lcm_pub_map.begin();
      while (iter != m_lcm_pub_map.end())
      {
        if (iter->second.updated)
        {
          iter->second.updated = false;
          topic_map[iter->first] = iter->second;
        }

        if (iter->second.ttime > std::chrono::steady_clock::time_point(std::chrono::microseconds(0)))
        {
          std::chrono::nanoseconds timeout = time - iter->second.ttime;
          if (timeout > std::chrono::milliseconds(CMN_REGISTRATION_TO))
          {
            iter = m_lcm_pub_map.erase(iter);
            if (iter == m_lcm_pub_map.end()) break;
          }
        }
        iter++;
      }
    }

    if (!g_monitoring()) return 0;

    // register lcm process "Any (LCM)"
    std::string host_name(eCAL::Process::GetHostName());
//    if (!topic_map.empty())
    {
      eCAL::pb::Sample ecal_process;
      auto ecal_process_mutable_process = ecal_process.mutable_process();
      ecal_process.set_cmd_type(eCAL::pb::bct_reg_process);
      ecal_process_mutable_process->set_hname(host_name);
      ecal_process_mutable_process->set_pname("Any (LCM)");
      ecal_process_mutable_process->set_uname("Any (LCM)");
      g_monitoring()->ApplySample(ecal_process);
    }

    // register lcm publisher
    eCAL::pb::Sample ecal_publisher;
    for (auto iter : topic_map)
    {
      ecal_publisher.Clear();
      ecal_publisher.set_cmd_type(eCAL::pb::bct_reg_publisher);
      auto ecal_publisher_mutable_topic = ecal_publisher.mutable_topic();
      ecal_publisher_mutable_topic->set_hname(host_name);
      ecal_publisher_mutable_topic->set_pname("Any (LCM)");
      ecal_publisher_mutable_topic->set_uname("Any (LCM)");
      ecal_publisher_mutable_topic->set_tname(iter.first);
      ecal_publisher_mutable_topic->set_tsize(iter.second.tsize);
      ecal_publisher_mutable_topic->set_ttype("lcm:binary");
      ecal_publisher_mutable_topic->set_dclock(iter.second.tclock);
      ecal_publisher_mutable_topic->set_dfreq(iter.second.dfreq);
      g_monitoring()->ApplySample(ecal_publisher);
    }

    return 0;
  }

  int CDataReaderLCM::Receive()
  {
    // wait for any incoming message
    struct sockaddr_in src_addr;
    size_t recv_len = m_receiver.Receive(m_msg_buffer.data(), m_msg_buffer.size(), 10, &src_addr);
    if (recv_len > 0)
    {
      lcmlite_receive_packet(m_lcm.get(), m_msg_buffer.data(), static_cast<int>(recv_len), src_addr.sin_addr.s_addr | ((uint64_t)src_addr.sin_port << 32)); //-V112
    }
    return 0;
  }
};
