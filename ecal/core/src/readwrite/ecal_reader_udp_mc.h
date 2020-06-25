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
 * @brief  eCAL udp multicast reader
**/

#pragma once

#include "readwrite/ecal_reader_udp.h"
#include "readwrite/ecal_reader_layer.h"

#include "ecal_config_hlp.h"
#include "ecal_thread.h"
#include "topic2mcast.h"

#include <memory>
#include <set>
#include <string>

namespace eCAL
{
  // ecal udp multicast data layer
  class CMulticastLayer : public CReaderLayer<CMulticastLayer>
  {
  public:
    CMulticastLayer() : started(false) {};
    ~CMulticastLayer()
    {
      thread.Stop();
    };

    void InitializeLayer()
    {
      SReceiverAttr attr;
      attr.ipaddr = eCALPAR(NET, UDP_MULTICAST_GROUP);
      attr.port = eCALPAR(NET, UDP_MULTICAST_PORT) + NET_UDP_MULTICAST_PORT_SAMPLE_OFF;
      attr.unicast = false;
      attr.loopback = true;
      attr.rcvbuf = eCALPAR(NET, UDP_MULTICAST_RCVBUF);
      rcv.Create(attr);
    }

    void StartLayer(std::string& topic_name_, QOS::SReaderQOS /*qos_*/)
    {
      if (!started)
      {
        thread.Start(0, std::bind(&CDataReaderUDP::Receive, &reader, &rcv));
        started = true;
      }
      // add topic name based multicast address
      std::string mcast_address = topic2mcast(topic_name_, eCALPAR(NET, UDP_MULTICAST_GROUP), eCALPAR(NET, UDP_MULTICAST_MASK));
      if (topic_name_mcast_set.find(mcast_address) == topic_name_mcast_set.end())
      {
        topic_name_mcast_set.insert(mcast_address);
        rcv.AddMultiCastGroup(mcast_address.c_str());
      }
    }

    void StopLayer(std::string& /*topic_name_*/)
    {
    }

    void ApplyLayerParameter(SReaderLayerPar& /*par_*/)
    {
    }

  private:
    bool                  started;
    CUDPReceiver          rcv;
    CThread               thread;
    CDataReaderUDP        reader;
    std::set<std::string> topic_name_mcast_set;
  };
};
