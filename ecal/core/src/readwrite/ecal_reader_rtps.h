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
 * @brief  eCAL rtps reader
**/

#pragma once

#include <ecal/ecal.h>

#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include "readwrite/ecal_reader_layer.h"
#include "readwrite/fastrtps/string_message_PubSubTypes.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace eCAL
{
  // ecal rtps reader
  class CDataReaderRtps
  {
  public:
    CDataReaderRtps();

    bool CreateRtpsSub(const std::string& topic_name_, const QOS::SReaderQOS& qos_);
    bool DestroyRtpsSub(const std::string& topic_name_);

  protected:
    // rtps subscriber
    class CSubscriberRtps
    {
    public:
      CSubscriberRtps(const std::string& topic_name_, QOS::SReaderQOS qos_);
      ~CSubscriberRtps();

      // rtps listener
      class SubListener :public eprosima::fastrtps::SubscriberListener
      {
      public:
        SubListener() : n_matched(0), n_samples(0) {};
        ~SubListener() {};

        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub_, eprosima::fastrtps::rtps::MatchingInfo& info_);
        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub_);

        string_message_                   m_string_msg;
        eprosima::fastrtps::SampleInfo_t  m_info;
        int                               n_matched;
        uint32_t                          n_samples;
      };

    protected:
      QOS::SReaderQOS                           m_qos;
      eprosima::fastrtps::Subscriber*           m_subscriber;
      class SubListener                         m_listener;
      string_message_PubSubType                 m_string_type;
      string_message_                           m_string_msg;
      eprosima::fastrtps::SubscriberAttributes  m_attr;
    };

    std::mutex                                         m_sub_map_sync;
    std::unordered_map<std::string, CSubscriberRtps*>  m_sub_map;
  };

  // ecal rtps data layer
  class CRtpsLayer : public CReaderLayer<CRtpsLayer>
  {
  public:
    CRtpsLayer() {};

    void InitializeLayer()
    {
    }

    void StartLayer(std::string& topic_name_, QOS::SReaderQOS qos_)
    {
      reader.CreateRtpsSub(topic_name_, qos_);
    }

    void StopLayer(std::string& topic_name_)
    {
      reader.DestroyRtpsSub(topic_name_);
    }

    void ApplyLayerParameter(SReaderLayerPar& /*par_*/)
    {
    }

  private:
    CDataReaderRtps reader;
  };
};
