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
 * @brief  tcp writer
**/

#include <ecal/ecal_config.h>

#include "serialization/ecal_serialize_sample_payload.h"

#include "ecal_writer_tcp.h"
#include "ecal_tcp_pubsub_logger.h"

#include "ecal_utils/portable_endian.h"

#include <cstring>

namespace eCAL
{
  std::mutex                            CDataWriterTCP::g_tcp_writer_executor_mtx;
  std::shared_ptr<tcp_pubsub::Executor> CDataWriterTCP::g_tcp_writer_executor;

  CDataWriterTCP::CDataWriterTCP(const std::string& host_name_, const std::string& topic_name_, const std::string& topic_id_, const Publisher::TCP::Configuration& tcp_config_) :
    m_config(tcp_config_)
  {
    {
      const std::lock_guard<std::mutex> lock(g_tcp_writer_executor_mtx);
      if (!g_tcp_writer_executor)
      {
        g_tcp_writer_executor = std::make_shared<tcp_pubsub::Executor>(Config::GetTcpPubsubWriterThreadpoolSize(), TcpPubsubLogger);
      }
    }

    // create publisher
    m_publisher = std::make_shared<tcp_pubsub::Publisher>(g_tcp_writer_executor);
    m_port      = m_publisher->getPort();

    // writer parameter
    m_host_name  = host_name_;
    m_topic_name = topic_name_;
    m_topic_id   = topic_id_;
  }

  SWriterInfo CDataWriterTCP::GetInfo()
  {
    SWriterInfo info_;

    info_.name           = "tcp";
    info_.description    = "tcp data writer";

    info_.has_mode_local = true;
    info_.has_mode_cloud = true;

    info_.send_size_max  = -1;

    return info_;
  }

  bool CDataWriterTCP::Write(const void* const buf_, const SWriterAttr& attr_)
  {
    if (!m_publisher) return false;

    // create new payload sample (header information only, no payload)
    Payload::Sample proto_header;
    auto& proto_header_topic = proto_header.topic;
    proto_header_topic.tname = m_topic_name;
    proto_header_topic.tid   = m_topic_id;

    // set payload content (without payload)
    auto& proto_header_content = proto_header.content;
    proto_header_content.id    = attr_.id;
    proto_header_content.clock = attr_.clock;
    proto_header_content.time  = attr_.time;
    proto_header_content.hash  = static_cast<int64_t>(attr_.hash);
    proto_header_content.size  = static_cast<int32_t>(attr_.len); // we use this size attribute for "header only"

    // Compute size of "ECAL" pre-header
    constexpr size_t ecal_magic_size(4 * sizeof(char));

    // Serialize payload sample
    std::vector<char> serialized_proto_header;
    SerializeToBuffer(proto_header, serialized_proto_header);

    // Get size of un-altered proto header size
    auto proto_header_size = static_cast<uint16_t>(serialized_proto_header.size());

    // Compute needed padding for aligning the payload
    constexpr size_t alignment_bytes     = 8;
    const     size_t minimal_header_size = ecal_magic_size +  sizeof(uint16_t)    +  proto_header_size;
    const     size_t padding_size        = (alignment_bytes - (minimal_header_size % alignment_bytes)) % alignment_bytes;

    // Add more bytes to the protobuf message to blow it up to the alignment
    // Aligning the user payload this way should be 100% compatible with previous
    // versions. It's most certainly bad style though and we should improve this 
    // in a future eCAL version.
    // 
    // TODO: REMOVE ME FOR ECAL6
    proto_header.padding.resize(padding_size);

    // Serialize payload sample again (now with padding) and reread size
    serialized_proto_header.clear();
    SerializeToBuffer(proto_header, serialized_proto_header);
    proto_header_size = static_cast<uint16_t>(serialized_proto_header.size());

    // prepare the header buffer
    //                    'ECAL'           + proto header size field  + proto header
    m_header_buffer.resize(ecal_magic_size + sizeof(uint16_t)         + proto_header_size);

    // add magic ecal header :-)
    m_header_buffer[0] = 'E';
    m_header_buffer[1] = 'C';
    m_header_buffer[2] = 'A';
    m_header_buffer[3] = 'L';

    // set proto header size right after magic ecal header
    *reinterpret_cast<uint16_t*>(&m_header_buffer[ecal_magic_size]) = htole16(proto_header_size);

    // copy serialized proto header right after sample size field
    memcpy((void*)(m_header_buffer.data() + ecal_magic_size + sizeof(uint16_t)), serialized_proto_header.data(), serialized_proto_header.size());

    // create tcp send buffer
    std::vector<std::pair<const char* const, const size_t>> send_vec;
    send_vec.reserve(2);

    // push header data
    send_vec.emplace_back(m_header_buffer.data(), m_header_buffer.size());
    // push payload data
    send_vec.emplace_back(static_cast<const char*>(buf_), attr_.len);

    // send it
    const bool success = m_publisher->send(send_vec);

    // return success
    return success;
  }

  Registration::ConnectionPar CDataWriterTCP::GetConnectionParameter()
  {
    Registration::ConnectionPar connection_par;
    connection_par.layer_par_tcp.port = m_port;
    return connection_par;
  }
}
