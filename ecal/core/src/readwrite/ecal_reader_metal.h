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
 * @brief  eCAL metal reader
**/

#pragma once

#include "ecal_expmap.h"

#include "io/rcv_sample.h"
#include "io/udp_receiver.h"

#include "ecal_config_hlp.h"
#include "ecal_thread.h"

#include "readwrite/ecal_reader_layer.h"
#include "readwrite/ecal_reader_udp.h"

#include <string>

namespace eCAL
{
  // ecal udp metal reader
  class CDataReaderMetal : public CDataReaderUDP
  {
  public:
    CDataReaderMetal();

    bool HasSample(const std::string& /* sample_name_ */) { return(true); };
    size_t ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType layer_);

  private:
    Util::CExpMap<std::string, std::string> m_pub_map;
  };

  // ecal udp metal data layer
  class CMetalLayer : public CReaderLayer<CMetalLayer>
  {
  public:
    CMetalLayer() : started(false) {};
    ~CMetalLayer()
    {
      thread.Stop();
    };

    void InitializeLayer()
    {
      SReceiverAttr attr;
      attr.ipaddr     = eCALPAR(NET, UDP_MULTICAST_GROUP_METAL);
      attr.port       = eCALPAR(NET, UDP_MULTICAST_PORT_METAL);
      attr.loopback   = true;
      attr.rcvbuf     = eCALPAR(NET, UDP_MULTICAST_RCVBUF);
      attr.local_only = false;
      rcv.Create(attr);
    }

    void StartLayer(std::string& /*topic_name_*/, QOS::SReaderQOS /*qos_*/)
    {
      if (started) return;
      thread.Start(0, std::bind(&CDataReaderUDP::Receive, &reader, &rcv));
      started = true;
    }

    void StopLayer(std::string& /*topic_name_*/)
    {
    }

    void ApplyLayerParameter(SReaderLayerPar& /*par_*/)
    {
    }

  private:
    bool              started;
    CUDPReceiver      rcv;
    CThread           thread;
    CDataReaderMetal  reader;
  };
};
