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
#include <memory>

#define TEST_RELIABLE    0
#define TEST_BEST_EFFORT 1

#define TEST_UDP         0
#define TEST_SHM         1
#define TEST_TCP         0
#define TEST_INPROC      0

auto g_snd_size (1000 * 1024);
auto g_pre_loops(100);
auto g_snd_loops( 1000 * 1000);

// subscriber callback function
std::atomic<size_t> g_callback_received;
void OnReceive(const char* /*topic_name_*/, const struct eCAL::SReceiveCallbackData* data_)
{
  g_callback_received += data_->size;
}

static std::string CreatePayLoad(size_t payload_size_)
{
  std::string s = "Hello World ";
  while (s.size() < payload_size_) s += s;
  s.resize(payload_size_);
  return(s);
}

bool test_throughput(int snd_size, int snd_loops, eCAL::TLayer::eTransportLayer layer, bool reliable)
{
  // internal timings for registration and data flow delay
  const int reg_time (2000);
  const int data_time(200);

  // create publisher for topic "ThroughPut"
  std::shared_ptr<eCAL::CPublisher> pub = std::make_shared<eCAL::CPublisher>();
  // set qos
  eCAL::QOS::SWriterQOS wqos;
  if (reliable) wqos.reliability = eCAL::QOS::reliable_reliability_qos;
  else          wqos.reliability = eCAL::QOS::best_effort_reliability_qos;
  pub->SetQOS(wqos);
  // set transport layer
  pub->SetLayerMode(eCAL::TLayer::tlayer_all, eCAL::TLayer::smode_off);
  pub->SetLayerMode(layer, eCAL::TLayer::smode_on);
  // create it
  pub->Create("ThroughPut");

  // create subscriber for topic "ThroughPut"
  std::shared_ptr<eCAL::CSubscriber> sub = std::make_shared<eCAL::CSubscriber>();
  // set qos
  eCAL::QOS::SReaderQOS rqos;
  if (reliable) rqos.reliability = eCAL::QOS::reliable_reliability_qos;
  else          rqos.reliability = eCAL::QOS::best_effort_reliability_qos;
  sub->SetQOS(rqos);
  // create it
  sub->Create("ThroughPut");
  // add callback
  sub->AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2));

  // let's match them
  eCAL::Process::SleepMS(reg_time);

  // default send string
  std::string send_s = CreatePayLoad(snd_size);

  // start time
  auto start = std::chrono::high_resolution_clock::now();

  // we send a few dummy loops to establish all connections
  for (auto i = 0; i < g_pre_loops; ++i)
  {
    pub->Send(send_s);
  }

  // let's finalize receive
  eCAL::Process::SleepMS(data_time);

  // reset received bytes counter
  g_callback_received = 0;

  // do some work
  size_t snd_bytes(0);
  for (auto i = 0; i < snd_loops; ++i)
  {
    size_t sent = pub->Send(send_s);
    snd_bytes += sent;
  }

  // end time
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Elapsed time : " << elapsed.count() << " s" << std::endl;

  // let's match them
  eCAL::Process::SleepMS(data_time);

  size_t sum_snd_bytes = send_s.size() * snd_loops;
  size_t sum_rcv_bytes = g_callback_received;
  std::cout << "Sent         : " << sum_snd_bytes << " bytes";
  std::cout << " (" << sum_snd_bytes / (1024 * 1024) << " MB)" << std::endl;
  std::cout << "Received     : " << sum_rcv_bytes << " bytes";
  std::cout << " (" << sum_rcv_bytes / (1024 * 1024) << " MB)" << std::endl;
  std::cout << "Lost         : " << sum_snd_bytes - sum_rcv_bytes << " bytes";
  std::cout << " (" << (sum_snd_bytes - sum_rcv_bytes) / (1024 * 1024) << " MB, ";
  std::cout << (sum_snd_bytes - sum_rcv_bytes) * 100.0f / sum_snd_bytes << " %)" << std::endl;
  std::cout << "Throughput   : " << int((sum_snd_bytes / (1024.0 * 1024.0)) / elapsed.count()) << " MB/s " << std::endl;

  // check receive and send bytes
  bool ret_state(false);
  if (reliable) ret_state = sum_snd_bytes         == sum_rcv_bytes;
  else          ret_state = (sum_snd_bytes * 3/4) <= sum_rcv_bytes;

  // destroy pub/sub
  pub->Destroy();
  sub->Destroy();

  return(ret_state);
}

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "pubsub_throughput");

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  bool success(false);

#if TEST_RELIABLE

#if TEST_UDP
  std::cout << "RELIABLE     : UDP" << std::endl;
  success = test_throughput(g_snd_size, g_snd_loops, eCAL::TLayer::tlayer_udp_mc, true);
  std::cout << "RESULT       : " << success << std::endl << std::endl;
#endif

#if TEST_SHM
  std::cout << "RELIABLE     : SHM" << std::endl;
  success = test_throughput(g_snd_size, g_snd_loops, eCAL::TLayer::tlayer_shm, true);
  std::cout << "RESULT       : " << success << std::endl << std::endl;
#endif

#if TEST_TCP
  std::cout << "RELIABLE     : TCP" << std::endl;
  success = test_throughput(4000, g_snd_loops, eCAL::TLayer::tlayer_tcp, true);
  std::cout << "RESULT       : " << success << std::endl << std::endl;
#endif

#if TEST_INPROC
  std::cout << "RELIABLE     : INPROC" << std::endl;
  success = test_throughput(4000, g_snd_loops, eCAL::TLayer::tlayer_inproc, true);
  std::cout << "RESULT       : " << success << std::endl << std::endl;
#endif

#endif // TEST_RELIABLE

#if TEST_BEST_EFFORT

#if TEST_UDP
  std::cout << "BEST EFFORT : UDP" << std::endl;
  success = test_throughput(g_snd_size, g_snd_loops, eCAL::TLayer::tlayer_udp_mc, false);
  std::cout << "RESULT       : " << success << std::endl << std::endl;
#endif

#if TEST_SHM
  std::cout << "BEST EFFORT : SHM" << std::endl;
  success = test_throughput(g_snd_size, g_snd_loops, eCAL::TLayer::tlayer_shm, false);
  std::cout << "RESULT       : " << success << std::endl << std::endl;
#endif

#if TEST_TCP
  std::cout << "BEST EFFORT : TCP" << std::endl;
  success = test_throughput(g_snd_size, g_snd_loops, eCAL::TLayer::tlayer_tcp, false);
  std::cout << "RESULT       : " << success << std::endl << std::endl;
#endif

#if TEST_INPROC
  std::cout << "BEST EFFORT : INPROC" << std::endl;
  success = test_throughput(4000, g_snd_loops, eCAL::TLayer::tlayer_inproc, false);
  std::cout << "RESULT       : " << success << std::endl << std::endl;
#endif

#endif // TEST_BEST_EFFORT

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
