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

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_reggate.h"

#include "mon/ecal_monitoring_def.h"
#include "pubsub/ecal_subgate.h"
#include "readwrite/ecal_reader_metal.h"

namespace eCAL
{
  template<> std::shared_ptr<CMetalLayer> CReaderLayer<CMetalLayer>::layer(nullptr);

  //////////////////////////////////////////////////////////////////
  // CMetalSampleReceiver
  //////////////////////////////////////////////////////////////////
  CDataReaderMetal::CDataReaderMetal()
  {
    // set expiration time for the metal publisher map
    // we use refresh registration time to "emulate" a metal
    // registration if the metal publisher is not using
    // eCAL metal RefreshRegistration API
    std::chrono::milliseconds registration_timeout(eCALPAR(CMN, REGISTRATION_REFRESH));
    m_pub_map.set_expiration(registration_timeout);
  }

  size_t CDataReaderMetal::ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType /*layer_*/)
  {
    switch (ecal_sample_.cmd_type())
    {
    case eCAL::pb::bct_reg_process:
    case eCAL::pb::bct_reg_publisher:
    case eCAL::pb::bct_reg_subscriber:
    {
      // okay the user decided to send registering informations
      // on the data layer, maybe there is a requirement to use
      // one socket for registration and data only
      // so we forward this sample to the registration and
      // monitoring gate
      if (g_reggate()) g_reggate()->ApplySample(ecal_sample_);
      if (g_monitoring()) g_monitoring()->ApplySample(ecal_sample_);
    }
    break;
    case eCAL::pb::bct_set_sample:
    {
      // (re)register external metal publication to activate data reader
      // this is needed if eCALMetal publishing application is not
      // serving registration layer
      // we use the expiration map because we do not have an
      // expiration set container available currently
      auto tname = ecal_sample_.topic().tname();
      auto iter = m_pub_map.find(tname);
      if (iter == m_pub_map.end())
      {
        m_pub_map[tname].clear();
        if (g_subgate()) g_subgate()->ApplyExtPubRegistration(ecal_sample_);
      }
      // remove timeout publications
      m_pub_map.remove_deprecated();

      // forward sample to common udp sample receiver
      return(CDataReaderUDP::ApplySample(ecal_sample_, eCAL::pb::eTLayerType::tl_ecal_udp_metal));
    }
    break;
    default:
      break;
    }
    return 0;
  }
};
