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

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace IO
{
  namespace UDP
  {
    size_t CreateSampleBuffer(const std::string& sample_name_, const std::vector<char>& serialized_sample_, std::vector<char>& payload_);

    using TransmitCallbackT = std::function<size_t(const void*, const size_t)>;
    size_t SendFragmentedMessage(char* buf_, size_t buf_len_, const TransmitCallbackT& transmit_cb_);
  }
}
