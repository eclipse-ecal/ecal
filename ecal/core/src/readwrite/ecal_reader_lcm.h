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

#pragma once

#include "io/rcv_sample.h"
#include "io/udp_receiver.h"
#include "readwrite/exttopic.h"
#include "readwrite/ecal_reader_layer.h"

#include "ecal_thread.h"

#include "lcm/lcmlite.h"

#include <mutex>
#include <unordered_map>

namespace eCAL
{
  // ecal udp lcm reader
  class CDataReaderLCM
  {
  public:
    CDataReaderLCM();

    int ApplyLcmPublisher(const std::string& topic_name_, size_t topic_size_);

    int Monitor();
    int Receive();

  protected:
    std::shared_ptr<lcmlite_sub_t>  m_lcm;
    lcmlite_subscription            m_lcm_subscription;
    std::vector<char>               m_msg_buffer;
    std::mutex                      m_lcm_pub_map_sync;
    ExtMapT                         m_lcm_pub_map;
    CUDPReceiver                    m_receiver;
  };

  // ecal udp lcm data layer
  class CLcmLayer : public CReaderLayer<CLcmLayer>
  {
  public:
    CLcmLayer() : initialzed(false), started(false) {};
    ~CLcmLayer()
    {
      mon_thread.Stop();
      rec_thread.Stop();
    };

    void InitializeLayer()
    {
      if (initialzed) return;
      // we need to start monitoring and data layer thread
      // because the monitoring information is on the data
      // layer in lcm
      mon_thread.Start(1000, std::bind(&CDataReaderLCM::Monitor, &reader));
      rec_thread.Start(0, std::bind(&CDataReaderLCM::Receive, &reader));
      initialzed = true;
    }

    void StartLayer(std::string& /*topic_name_*/, QOS::SReaderQOS /*qos_*/)
    {
      if (started) return;
      started = true;
    }

    void StopLayer(std::string& /*topic_name_*/)
    {
    }

    void ApplyLayerParameter(SReaderLayerPar& /*par_*/)
    {
    }

  private:
    bool            initialzed;
    bool            started;
    CThread         mon_thread;
    CThread         rec_thread;
    CDataReaderLCM  reader;
  };
};
