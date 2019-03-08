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
 * @brief  rtps data writer
**/

#pragma once

#include <string>

#include <ecal/ecal_qos.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4146 4800)
#endif
#include "ecal/pb/ecal.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "readwrite/ecal_writer_base.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>

#include "readwrite/fastrtps/string_message_PubSubTypes.h"

namespace eCAL
{
  class CDataWriterRTPS : public CDataWriterBase
  {
  public:
    CDataWriterRTPS();
    ~CDataWriterRTPS();

    void GetInfo(SWriterInfo info_) override;

    bool Create(const std::string& host_name_, const std::string& topic_name_, const std::string & topic_id_) override;
    bool Destroy() override;

    bool SetQOS(const QOS::SWriterQOS& qos_) override;

    size_t Send(const SWriterData& data_) override;

  protected:
    bool CreateRtpsPub(const std::string& topic_name_);
    bool DestroyRtpsPub();

    class PubListener :public eprosima::fastrtps::PublisherListener
    {
    public:
      PubListener() :n_matched(0) {};
      ~PubListener() {};
      void onPublicationMatched(eprosima::fastrtps::Publisher* pub_, eprosima::fastrtps::rtps::MatchingInfo& info_);
      int n_matched;
    };

    eprosima::fastrtps::Publisher*           m_publisher;
    class PubListener                        m_listener;
    string_message_PubSubType                m_string_type;
    string_message_                          m_string_msg;
    eprosima::fastrtps::PublisherAttributes  m_attr;
  };
}
