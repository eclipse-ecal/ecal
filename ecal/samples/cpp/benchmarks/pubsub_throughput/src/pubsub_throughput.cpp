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

#include <ecal/ecal.h>

#include <atomic>
#include <chrono>
#include <iostream>

#include "binary_payload_writer.h"

const auto g_snd_size (8* 1024 * 1024);
const auto g_snd_loops(1000);

void throughput_test(int snd_size, int snd_loops, eCAL::TLayer::eTransportLayer layer, bool zero_copy)
{
  // create payload
  CBinaryPayload payload(snd_size);

  // create publisher config
  eCAL::Publisher::Configuration pub_config;

  // set transport layer
  pub_config.layer.shm.enable = false;
  pub_config.layer.udp.enable = false;
  pub_config.layer.tcp.enable = false;
  switch (layer)
  {
  case eCAL::TLayer::tlayer_shm:
    pub_config.layer.shm.enable = true;
    break;
  case eCAL::TLayer::tlayer_udp_mc:
    pub_config.layer.udp.enable = true;
    break;
  case eCAL::TLayer::tlayer_tcp:
    pub_config.layer.tcp.enable = true;
    break;
  }

  // enable zero copy mode
  pub_config.layer.shm.zero_copy_mode = zero_copy;

  // enable handshake mode
  pub_config.layer.shm.acknowledge_timeout_ms = 100;

  // create publisher
  eCAL::CPublisher pub("throughput", pub_config);

  // create subscriber
  eCAL::CSubscriber sub("throughput");
  // add callback
  std::atomic<size_t> received_bytes;
  auto on_receive = [&](const struct eCAL::SReceiveCallbackData* data_) {
    received_bytes += data_->size;
  };
  sub.AddReceiveCallback(std::bind(on_receive, std::placeholders::_2));

  // let's match them
  eCAL::Process::SleepMS(2000);

  // initial call to allocate memory file
  pub.Send(payload);

  // reset received bytes counter
  received_bytes = 0;

  // start time
  auto start = std::chrono::high_resolution_clock::now();

  // do some work
  for (auto i = 0; i < snd_loops; ++i)
  {
    pub.Send(payload);
  }

  // end time
  auto finish = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Elapsed time : " << elapsed.count() << " s" << '\n';

  const size_t sum_snd_bytes = payload.GetSize() * snd_loops;
  const size_t sum_rcv_bytes = received_bytes;
  std::cout << "Sent         : " << sum_snd_bytes / (1024 * 1024) << " MB" << '\n';
  std::cout << "Received     : " << sum_rcv_bytes / (1024 * 1024) << " MB" << '\n';
  std::cout << "Lost         : " << sum_snd_bytes - sum_rcv_bytes << " bytes";
  std::cout << " (" << (sum_snd_bytes - sum_rcv_bytes) / (1024 * 1024) << " MB, ";
  std::cout << (sum_snd_bytes - sum_rcv_bytes) * 100.0f / sum_snd_bytes << " %)" << '\n';
  std::cout << "Throughput   : " << int((sum_snd_bytes / (1024.0 * 1024.0)) / elapsed.count()) << " MB/s " << '\n';
  std::cout << "Throughput   : " << int((sum_snd_bytes / (1024.0 * 1024.0 * 1024.0)) / elapsed.count()) << " GB/s " << '\n';
}

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "pubsub_throughput");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  std::cout << "---------------------------" << '\n';
  std::cout << "LAYER: SHM"                  << '\n';
  std::cout << "---------------------------" << '\n';
  throughput_test(g_snd_size, g_snd_loops, eCAL::TLayer::tlayer_shm, false);
  std::cout << '\n' << '\n';

  std::cout << "---------------------------" << '\n';
  std::cout << "LAYER: SHM ZERO-COPY"        << '\n';
  std::cout << "---------------------------" << '\n';
  throughput_test(g_snd_size, g_snd_loops, eCAL::TLayer::tlayer_shm, true);
  std::cout << '\n' << '\n';

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
