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
 * @brief  UDP sample sender to send messages of type eCAL::Sample
**/

#pragma once

#include "io/udp/ecal_udp_sender_attr.h"

#include <ecaludp/socket.h>

#include <string>
#include <vector>

namespace eCAL
{
  namespace UDP
  {
    class CSampleSender
    {
    public:
      explicit CSampleSender(const SSenderAttr& attr_);
      virtual ~CSampleSender();

      size_t Send(const std::string& sample_name_, const std::vector<char>& serialized_sample_);

    private:
      void InitializeSocket(const SSenderAttr& attr_);

      std::unique_ptr<asio::io_context>       m_io_context;
      std::unique_ptr<ecaludp::Socket>        m_socket;
      asio::ip::udp::endpoint                 m_destination_endpoint;
    };
  }
}
