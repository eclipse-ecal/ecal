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
 * @brief  raw message buffer handling
**/

#include "snd_fragments.h"
#include "msg_type.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace
{
  // random number generator
  unsigned long xorshf96(unsigned long& x, unsigned long& y, unsigned long& z)  // period 2^96-1
  {
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    unsigned long t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
  }
}

namespace IO
{
  namespace UDP
  {
    size_t CreateSampleBuffer(const std::string& sample_name_, const std::vector<char>& serialized_sample_, std::vector<char>& payload_)
    {
      const unsigned short sample_name_size = (unsigned short)sample_name_.size() + 1;
      const size_t   sample_size = serialized_sample_.size();
      const size_t   data_size   = sizeof(sample_name_size) + sample_name_size + sample_size;

      // create payload buffer with reserved space for first message head
      payload_.resize(data_size + sizeof(struct SUDPMessageHead));
      char* payload_data = payload_.data() + sizeof(struct SUDPMessageHead);

      // write topic name size
      ((unsigned short*)payload_data)[0] = sample_name_size;
      // write topic name
      memcpy(payload_data + sizeof(sample_name_size), sample_name_.c_str(), sample_name_size);
      // write payload
      memcpy(payload_data + sizeof(sample_name_size) + sample_name_size, serialized_sample_.data(), sample_size);

      return data_size;
    }

    size_t SendFragmentedMessage(char* buf_, size_t buf_len_, const TransmitCallbackT& transmit_cb_)
    {
      if (buf_ == nullptr) return(0);

      size_t sent_sum(0);

      auto total_packet_num = int32_t(buf_len_ / MSG_PAYLOAD_SIZE);
      if (buf_len_ % MSG_PAYLOAD_SIZE) total_packet_num++;

      // create message header
      struct SUDPMessageHead msg_header;

      switch (total_packet_num)
      {
      case 1:
      {
        // create start packet
        msg_header.type = msg_type_header_with_content;
        msg_header.id = -1;  // not needed for combined header / data message
        msg_header.num = 1;
        msg_header.len = int32_t(buf_len_);

        // copy msg_header in send buffer
        memcpy(buf_, &msg_header, sizeof(struct SUDPMessageHead));

        // send single header + data package
        size_t sent = transmit_cb_(buf_, sizeof(struct SUDPMessageHead) + buf_len_);
        if (sent == 0) return(sent);
        sent_sum += sent;

#ifndef NDEBUG
        // log it
        //std::cout << "SendRawBuffer Packet Sent - HEADER_WITH_CONTENT (" + std::to_string(sent) + " Bytes)" << std::endl;
#endif
      }
      break;
      default:
      {
        // create start package
        msg_header.type = msg_type_header;
        {
          // create random number for message id
          {
            static std::mutex xorshf96_mtx;
            const std::lock_guard<std::mutex> lock(xorshf96_mtx);

            static unsigned long x = static_cast<unsigned long>(std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch()).count()
              );
            static unsigned long y = 362436069;
            static unsigned long z = 521288629;

            msg_header.id = static_cast<int32_t>(xorshf96(x, y, z));
          }
        }
        msg_header.num = total_packet_num;
        msg_header.len = int32_t(buf_len_);

        // send start package
        size_t sent = transmit_cb_(&msg_header, sizeof(struct SUDPMessageHead));
        if (sent == 0) return(sent);
        sent_sum += sent;

#ifndef NDEBUG
        // log it
        //std::cout << "SendRawBuffer Packet Sent - HEADER (" + std::to_string(sent) + " Bytes)" << std::endl;
#endif

      // send data packages
        msg_header.type = msg_type_content;
        for (int32_t current_packet_num = 0; current_packet_num < total_packet_num; current_packet_num++)
        {
          // calculate current payload
          size_t current_snd_len = buf_len_;
          if (current_snd_len > MSG_PAYLOAD_SIZE) current_snd_len = MSG_PAYLOAD_SIZE;

          if (current_snd_len > 0)
          {
            // reduce total send len
            buf_len_ -= current_snd_len;

            // create data packet numbering
            msg_header.num = current_packet_num;
            msg_header.len = int32_t(current_snd_len);

            // copy msg_header in send buffer
            memcpy(buf_ + static_cast<size_t>(current_packet_num) * MSG_PAYLOAD_SIZE, &msg_header, sizeof(struct SUDPMessageHead));

            // send data package
            sent = transmit_cb_(buf_ + static_cast<size_t>(current_packet_num) * MSG_PAYLOAD_SIZE, sizeof(struct SUDPMessageHead) + current_snd_len);
            if (sent == 0) return(sent);

#ifndef NDEBUG
            // log it
            //std::cout << "SendRawBuffer Packet Sent - CONTENT (" + std::to_string(sent) + " Bytes)" << std::endl;
#endif
            sent_sum += sent;
          }
        }
      }
      break;
      }

      return(sent_sum);
    }
  }
}
